import json

from fastapi import APIRouter, Depends, File, Form, HTTPException, UploadFile
from fastapi.responses import StreamingResponse
from sqlalchemy.orm import Session

from app.db.models import Conversation, DocumentRecord, KnowledgeBaseRecord, Message, User
from app.db.session import get_db
from app.models.schemas import (
    ConversationCreateRequest,
    ConversationResponse,
    ConversationUpdateRequest,
    DeleteResponse,
    DocumentResponse,
    KnowledgeBaseIngestRequest,
    KnowledgeBaseIngestResponse,
    KnowledgeBaseResponse,
    MessageResponse,
    RagCitation,
    RagQueryRequest,
    RagQueryResponse,
    RagSource,
    StoredChatRequest,
    StoredChatResponse,
    UsageResponse,
)
from app.rag.service import (
    build_rag_context,
    index_document_chunks,
    index_knowledge_base_chunks,
    search_similar_chunks,
)
from app.services.ai_service import (
    ask_legal_assistant_text,
    ask_with_rag,
    analyze_text,
    settings,
    stream_plain_chat,
    stream_with_rag,
)
from app.services.auth_service import get_current_user
from app.services.conversation_service import (
    add_message,
    create_conversation,
    delete_conversation,
    get_user_conversation,
    update_conversation_title,
)
from app.services.demo_service import consume_demo_request, get_usage_status
from app.services.document_service import ensure_size_limit, parse_upload_file, persist_uploaded_file
from app.services.pravo_ingest_service import ingest_pravo_url

router = APIRouter(prefix="/v1")


@router.post("/conversations", response_model=ConversationResponse)
def create_user_conversation(
    payload: ConversationCreateRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> ConversationResponse:
    conversation = create_conversation(db, current_user, payload.title)
    return ConversationResponse(
        id=conversation.id,
        title=conversation.title,
        created_at=conversation.created_at.isoformat(),
        updated_at=conversation.updated_at.isoformat(),
    )


@router.get("/conversations", response_model=list[ConversationResponse])
def list_conversations(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> list[ConversationResponse]:
    conversations = (
        db.query(Conversation)
        .filter(Conversation.user_id == current_user.id)
        .order_by(Conversation.updated_at.desc())
        .all()
    )
    return [
        ConversationResponse(
            id=item.id,
            title=item.title,
            created_at=item.created_at.isoformat(),
            updated_at=item.updated_at.isoformat(),
        )
        for item in conversations
    ]


@router.patch("/conversations/{conversation_id}", response_model=ConversationResponse)
def rename_conversation(
    conversation_id: int,
    payload: ConversationUpdateRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> ConversationResponse:
    conversation = get_user_conversation(db, current_user, conversation_id)
    if not conversation:
        raise HTTPException(status_code=404, detail="Диалог не найден.")

    conversation = update_conversation_title(db, conversation, payload.title)
    return ConversationResponse(
        id=conversation.id,
        title=conversation.title,
        created_at=conversation.created_at.isoformat(),
        updated_at=conversation.updated_at.isoformat(),
    )


@router.delete("/conversations/{conversation_id}", response_model=DeleteResponse)
def remove_conversation(
    conversation_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> DeleteResponse:
    conversation = get_user_conversation(db, current_user, conversation_id)
    if not conversation:
        raise HTTPException(status_code=404, detail="Диалог не найден.")

    delete_conversation(db, conversation)
    return DeleteResponse(status="ok", detail="Диалог удалён.")


@router.get("/conversations/{conversation_id}/messages", response_model=list[MessageResponse])
def list_messages(
    conversation_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> list[MessageResponse]:
    conversation = get_user_conversation(db, current_user, conversation_id)
    if not conversation:
        raise HTTPException(status_code=404, detail="Диалог не найден.")

    messages = (
        db.query(Message)
        .filter(Message.conversation_id == conversation.id)
        .order_by(Message.created_at.asc())
        .all()
    )
    return [
        MessageResponse(
            id=item.id,
            role=item.role,
            content=item.content,
            created_at=item.created_at.isoformat(),
        )
        for item in messages
    ]


@router.post("/chat", response_model=StoredChatResponse)
def chat_and_store(
    payload: StoredChatRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> StoredChatResponse:
    consume_demo_request(db, current_user)

    if payload.conversation_id is not None:
        conversation = get_user_conversation(db, current_user, payload.conversation_id)
        if not conversation:
            raise HTTPException(status_code=404, detail="Диалог не найден.")
    else:
        conversation = create_conversation(db, current_user, payload.title or payload.message[:60])

    add_message(db, conversation, "user", payload.message)
    answer = ask_legal_assistant_text(payload.message, payload.context)
    add_message(db, conversation, "assistant", answer)

    return StoredChatResponse(
        answer=answer,
        model=settings.openai_model,
        conversation_id=conversation.id,
        disclaimer=(
            "Ответ носит информационный характер и не заменяет очную консультацию "
            "с лицензированным юристом."
        ),
    )


@router.post("/chat/stream")
def chat_stream(
    payload: StoredChatRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> StreamingResponse:
    consume_demo_request(db, current_user)

    if payload.conversation_id is not None:
        conversation = get_user_conversation(db, current_user, payload.conversation_id)
        if not conversation:
            raise HTTPException(status_code=404, detail="Диалог не найден.")
    else:
        conversation = create_conversation(db, current_user, payload.title or payload.message[:60])

    add_message(db, conversation, "user", payload.message)

    def event_stream():
        yield (
            "event: meta\ndata: "
            f"{json.dumps({'conversation_id': conversation.id, 'model': settings.openai_model}, ensure_ascii=False)}\n\n"
        )
        answer_parts: list[str] = []
        for chunk in stream_plain_chat(payload.message, payload.context):
            if chunk.startswith("event: delta"):
                try:
                    payload_json = chunk.split("data: ", 1)[1].strip()
                    delta_payload = json.loads(payload_json)
                    answer_parts.append(delta_payload.get('delta', ''))
                except Exception:
                    pass
            yield chunk
        final_answer = "".join(answer_parts).strip()
        if final_answer:
            add_message(db, conversation, "assistant", final_answer)

    return StreamingResponse(event_stream(), media_type="text/event-stream")


@router.post("/documents/upload", response_model=DocumentResponse)
async def upload_document(
    file: UploadFile = File(...),
    analysis_type: str = Form("general"),
    question: str | None = Form(None),
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> DocumentResponse:
    file_bytes = await file.read()
    ensure_size_limit(file_bytes)

    try:
        extracted_text, extracted_via, extension = parse_upload_file(
            filename=file.filename or "",
            content_type=file.content_type,
            file_bytes=file_bytes,
        )
        storage_path = persist_uploaded_file(file.filename or "document", file_bytes)
        analysis_summary = analyze_text(
            analysis_type=analysis_type,
            document_text=extracted_text,
            question=question,
            filename=file.filename,
        )
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка загрузки документа: {exc}") from exc

    document = DocumentRecord(
        user_id=current_user.id,
        filename=file.filename or "unknown",
        extension=extension,
        content_type=file.content_type,
        storage_path=storage_path,
        extracted_via=extracted_via,
        extracted_text=extracted_text,
        analysis_summary=analysis_summary,
        indexed_in_rag="no",
    )
    db.add(document)
    db.commit()
    db.refresh(document)

    try:
        indexed_count = index_document_chunks(
            db=db,
            user_id=current_user.id,
            document_id=document.id,
            filename=document.filename,
            text=document.extracted_text,
        )
        document.indexed_in_rag = "yes" if indexed_count > 0 else "no"
    except Exception:
        document.indexed_in_rag = "error"

    db.add(document)
    db.commit()
    db.refresh(document)

    return DocumentResponse(
        id=document.id,
        filename=document.filename,
        extension=document.extension,
        content_type=document.content_type,
        extracted_via=document.extracted_via,
        extracted_text=document.extracted_text,
        analysis_summary=document.analysis_summary,
        indexed_in_rag=document.indexed_in_rag,
        created_at=document.created_at.isoformat(),
    )


@router.get("/documents", response_model=list[DocumentResponse])
def list_documents(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> list[DocumentResponse]:
    documents = (
        db.query(DocumentRecord)
        .filter(DocumentRecord.user_id == current_user.id)
        .order_by(DocumentRecord.created_at.desc())
        .all()
    )
    return [
        DocumentResponse(
            id=item.id,
            filename=item.filename,
            extension=item.extension,
            content_type=item.content_type,
            extracted_via=item.extracted_via,
            extracted_text=item.extracted_text,
            analysis_summary=item.analysis_summary,
            indexed_in_rag=item.indexed_in_rag,
            created_at=item.created_at.isoformat(),
        )
        for item in documents
    ]


@router.get("/documents/{document_id}", response_model=DocumentResponse)
def get_document(
    document_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> DocumentResponse:
    document = (
        db.query(DocumentRecord)
        .filter(DocumentRecord.id == document_id, DocumentRecord.user_id == current_user.id)
        .first()
    )
    if not document:
        raise HTTPException(status_code=404, detail="Документ не найден.")

    return DocumentResponse(
        id=document.id,
        filename=document.filename,
        extension=document.extension,
        content_type=document.content_type,
        extracted_via=document.extracted_via,
        extracted_text=document.extracted_text,
        analysis_summary=document.analysis_summary,
        indexed_in_rag=document.indexed_in_rag,
        created_at=document.created_at.isoformat(),
    )


@router.delete("/documents/{document_id}", response_model=DeleteResponse)
def delete_document(
    document_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> DeleteResponse:
    document = (
        db.query(DocumentRecord)
        .filter(DocumentRecord.id == document_id, DocumentRecord.user_id == current_user.id)
        .first()
    )
    if not document:
        raise HTTPException(status_code=404, detail="Документ не найден.")

    db.delete(document)
    db.commit()
    return DeleteResponse(status="ok", detail="Документ удалён.")


@router.post("/knowledge-base/ingest", response_model=KnowledgeBaseIngestResponse)
def ingest_knowledge_base(
    payload: KnowledgeBaseIngestRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> KnowledgeBaseIngestResponse:
    try:
        ingested, skipped = ingest_pravo_url(db, payload.url, payload.max_documents)
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка ingestion pravo.gov.ru: {exc}") from exc

    response_items: list[KnowledgeBaseResponse] = []
    for record in ingested:
        try:
            indexed_count = index_knowledge_base_chunks(
                db=db,
                knowledge_base_id=record.id,
                title=record.title,
                source_url=record.source_url,
                text=record.extracted_text,
            )
            record.indexed_in_rag = "yes" if indexed_count > 0 else "no"
        except Exception:
            record.indexed_in_rag = "error"
        db.add(record)
        db.commit()
        db.refresh(record)

        response_items.append(
            KnowledgeBaseResponse(
                id=record.id,
                title=record.title,
                source_url=record.source_url,
                source_type=record.source_type,
                filename=record.filename,
                indexed_in_rag=record.indexed_in_rag,
                created_at=record.created_at.isoformat(),
            )
        )

    return KnowledgeBaseIngestResponse(ingested=response_items, skipped=skipped)


@router.get("/knowledge-base", response_model=list[KnowledgeBaseResponse])
def list_knowledge_base(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> list[KnowledgeBaseResponse]:
    records = db.query(KnowledgeBaseRecord).order_by(KnowledgeBaseRecord.created_at.desc()).all()
    return [
        KnowledgeBaseResponse(
            id=item.id,
            title=item.title,
            source_url=item.source_url,
            source_type=item.source_type,
            filename=item.filename,
            indexed_in_rag=item.indexed_in_rag,
            created_at=item.created_at.isoformat(),
        )
        for item in records
    ]


@router.get("/usage", response_model=UsageResponse)
def get_demo_usage(
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> UsageResponse:
    return get_usage_status(db, current_user)


@router.post("/rag/query", response_model=RagQueryResponse)
def rag_query(
    payload: RagQueryRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> RagQueryResponse:
    consume_demo_request(db, current_user)

    if payload.conversation_id is not None:
        conversation = get_user_conversation(db, current_user, payload.conversation_id)
        if not conversation:
            raise HTTPException(status_code=404, detail="Диалог не найден.")
    else:
        conversation = create_conversation(db, current_user, payload.title or payload.question[:60])

    sources = search_similar_chunks(
        db=db,
        user_id=current_user.id,
        question=payload.question,
        top_k=payload.top_k,
        search_scope=payload.search_scope,
    )
    rag_context = build_rag_context(sources)

    add_message(db, conversation, "user", payload.question)
    structured = ask_with_rag(payload.question, rag_context)
    add_message(db, conversation, "assistant", structured.answer)

    return RagQueryResponse(
        answer=structured.answer,
        model=settings.openai_model,
        conversation_id=conversation.id,
        sources=[RagSource(**source) for source in sources],
        citations=[RagCitation(**citation.model_dump()) for citation in structured.citations],
        confidence_score=structured.confidence_score,
        refusal_reason=structured.refusal_reason,
        source_validation=structured.source_validation,
        disclaimer=(
            "Ответ носит информационный характер и не заменяет очную консультацию "
            "с лицензированным юристом."
        ),
    )


@router.post("/rag/stream")
def rag_stream(
    payload: RagQueryRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> StreamingResponse:
    consume_demo_request(db, current_user)

    if payload.conversation_id is not None:
        conversation = get_user_conversation(db, current_user, payload.conversation_id)
        if not conversation:
            raise HTTPException(status_code=404, detail="Диалог не найден.")
    else:
        conversation = create_conversation(db, current_user, payload.title or payload.question[:60])

    sources = search_similar_chunks(
        db=db,
        user_id=current_user.id,
        question=payload.question,
        top_k=payload.top_k,
        search_scope=payload.search_scope,
    )
    rag_context = build_rag_context(sources)
    add_message(db, conversation, "user", payload.question)

    def event_stream():
        answer_parts: list[str] = []
        meta = {
            "conversation_id": conversation.id,
            "sources": sources,
            "model": settings.openai_model,
            "search_scope": payload.search_scope,
        }
        yield f"event: meta\ndata: {json.dumps(meta, ensure_ascii=False)}\n\n"

        result_payload: dict | None = None
        for chunk in stream_with_rag(payload.question, rag_context):
            if chunk.startswith("event: delta"):
                try:
                    payload_json = chunk.split("data: ", 1)[1].strip()
                    delta_payload = json.loads(payload_json)
                    answer_parts.append(delta_payload.get("delta", ""))
                except Exception:
                    pass
            elif chunk.startswith("event: result"):
                try:
                    payload_json = chunk.split("data: ", 1)[1].strip()
                    result_payload = json.loads(payload_json)
                except Exception:
                    result_payload = None
            yield chunk

        final_answer = "".join(answer_parts).strip()
        if final_answer:
            add_message(db, conversation, "assistant", final_answer)

        if result_payload is not None:
            yield f"event: legal_result\ndata: {json.dumps(result_payload, ensure_ascii=False)}\n\n"

    return StreamingResponse(event_stream(), media_type="text/event-stream")
