import json
from collections.abc import Generator

from fastapi import HTTPException
from openai import APIConnectionError, APIStatusError, AuthenticationError, OpenAI

from app.config import get_settings
from app.models.schemas import ChatRequest, ChatResponse, RagStructuredAnswer
from app.services.prompt_service import build_analysis_prompt, build_rag_prompt, build_system_prompt

settings = get_settings()


def create_response_or_raise(client: OpenAI, *, instructions: str, input_text: str):
    try:
        return client.responses.create(
            model=settings.openai_model,
            instructions=instructions,
            input=input_text,
        )
    except AuthenticationError as exc:
        raise HTTPException(
            status_code=502,
            detail="Ошибка авторизации OpenAI. Проверьте OPENAI_API_KEY в Railway Variables.",
        ) from exc
    except APIConnectionError as exc:
        cause = exc.__cause__ or exc
        raise HTTPException(
            status_code=502,
            detail=(
                "Сервер не может подключиться к OpenAI API. "
                f"Техническая причина: {cause}"
            ),
        ) from exc
    except APIStatusError as exc:
        raise HTTPException(
            status_code=502,
            detail=f"OpenAI API вернул ошибку: {exc.status_code}.",
        ) from exc


def get_client() -> OpenAI:
    if not settings.openai_api_key:
        raise HTTPException(
            status_code=500,
            detail="OPENAI_API_KEY не задан. Добавьте ключ в backend/.env файл.",
        )
    return OpenAI(api_key=settings.openai_api_key)


def extract_text(response) -> str:
    text = getattr(response, "output_text", None)
    if text:
        return text
    return "Модель не вернула текстовый ответ."


def parse_json_object(text: str) -> dict:
    text = text.strip()
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        start = text.find("{")
        end = text.rfind("}")
        if start != -1 and end != -1 and end > start:
            return json.loads(text[start : end + 1])
        raise


def ask_legal_assistant(payload: ChatRequest) -> ChatResponse:
    client = get_client()
    user_input = payload.message
    if payload.context:
        user_input += f"\n\nКонтекст пользователя:\n{payload.context}"

    response = create_response_or_raise(
        client,
        instructions=build_system_prompt(),
        input_text=user_input,
    )

    return ChatResponse(
        answer=extract_text(response),
        model=settings.openai_model,
        conversation_id=payload.conversation_id,
        disclaimer=(
            "Ответ носит информационный характер и не заменяет очную консультацию "
            "с лицензированным юристом."
        ),
    )


def ask_legal_assistant_text(message: str, context: str | None = None) -> str:
    return ask_legal_assistant(ChatRequest(message=message, context=context)).answer


def stream_plain_chat(message: str, context: str | None = None) -> Generator[str, None, None]:
    answer = ask_legal_assistant_text(message, context)
    for index in range(0, len(answer), 80):
        delta = answer[index : index + 80]
        if delta:
            yield f"event: delta\ndata: {json.dumps({'delta': delta}, ensure_ascii=False)}\n\n"
    yield "event: done\ndata: {\"status\":\"completed\"}\n\n"


def ask_with_rag(question: str, rag_context: str) -> RagStructuredAnswer:
    client = get_client()
    response = create_response_or_raise(
        client,
        instructions=build_system_prompt(),
        input_text=build_rag_prompt(question, rag_context),
    )
    parsed = parse_json_object(extract_text(response))
    result = RagStructuredAnswer(**parsed)
    result.confidence_score = max(0.0, min(1.0, result.confidence_score))
    return result


def stream_with_rag(question: str, rag_context: str) -> Generator[str, None, None]:
    structured = ask_with_rag(question, rag_context)
    for index in range(0, len(structured.answer), 80):
        delta = structured.answer[index : index + 80]
        if delta:
            yield f"event: delta\ndata: {json.dumps({'delta': delta}, ensure_ascii=False)}\n\n"

    meta = {
        "citations": [citation.model_dump() for citation in structured.citations],
        "confidence_score": structured.confidence_score,
        "refusal_reason": structured.refusal_reason,
        "source_validation": structured.source_validation,
    }
    yield f"event: result\ndata: {json.dumps(meta, ensure_ascii=False)}\n\n"
    yield "event: done\ndata: {\"status\":\"completed\"}\n\n"


def analyze_text(
    *,
    analysis_type: str,
    document_text: str,
    question: str | None,
    filename: str | None = None,
) -> str:
    client = get_client()
    prompt = build_analysis_prompt(
        analysis_type=analysis_type,
        document_text=document_text,
        question=question,
        filename=filename,
    )
    response = create_response_or_raise(
        client,
        instructions=build_system_prompt(),
        input_text=prompt,
    )
    return extract_text(response)
