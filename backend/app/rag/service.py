from __future__ import annotations

import json
import math
import re

from sqlalchemy.orm import Session

from app.config import get_settings
from app.db.models import KnowledgeBaseChunkRecord, RagChunkRecord
from app.services.ai_service import get_client

settings = get_settings()

ARTICLE_SPLIT_PATTERN = re.compile(
    r"(?im)^(?:статья|ст\.|раздел|глава|chapter|section)\s+[\w\d\-\.]+.*$"
)


def split_by_legal_sections(text: str) -> list[str]:
    matches = list(ARTICLE_SPLIT_PATTERN.finditer(text))
    if not matches:
        return []

    parts: list[str] = []
    for index, match in enumerate(matches):
        start = match.start()
        end = matches[index + 1].start() if index + 1 < len(matches) else len(text)
        part = text[start:end].strip()
        if part:
            parts.append(part)
    return parts


def split_large_section(section: str) -> list[str]:
    if len(section) <= settings.rag_chunk_size:
        return [section]

    chunks: list[str] = []
    step = max(1, settings.rag_chunk_size - settings.rag_chunk_overlap)
    start = 0
    while start < len(section):
        chunk = section[start : start + settings.rag_chunk_size].strip()
        if chunk:
            chunks.append(chunk)
        start += step
    return chunks


def chunk_text(text: str) -> list[str]:
    text = text.strip()
    if not text:
        return []

    legal_sections = split_by_legal_sections(text)
    if legal_sections:
        chunks: list[str] = []
        for section in legal_sections:
            chunks.extend(split_large_section(section))
        return chunks

    paragraphs = [part.strip() for part in text.split("\n\n") if part.strip()]
    if paragraphs:
        chunks = []
        current = ""
        for paragraph in paragraphs:
            candidate = f"{current}\n\n{paragraph}".strip() if current else paragraph
            if len(candidate) <= settings.rag_chunk_size:
                current = candidate
                continue
            if current:
                chunks.append(current)
            if len(paragraph) <= settings.rag_chunk_size:
                current = paragraph
            else:
                chunks.extend(split_large_section(paragraph))
                current = ""
        if current:
            chunks.append(current)
        if chunks:
            return chunks

    return split_large_section(text)


def embed_texts(texts: list[str]) -> list[list[float]]:
    if not texts:
        return []

    client = get_client()
    response = client.embeddings.create(
        model=settings.embedding_model,
        input=texts,
        dimensions=settings.embedding_dimensions,
    )
    return [item.embedding for item in response.data]


def cosine_similarity(a: list[float], b: list[float]) -> float:
    dot = sum(x * y for x, y in zip(a, b, strict=False))
    norm_a = math.sqrt(sum(x * x for x in a))
    norm_b = math.sqrt(sum(y * y for y in b))
    if norm_a == 0 or norm_b == 0:
        return 0.0
    return dot / (norm_a * norm_b)


def index_document_chunks(
    db: Session,
    user_id: int,
    document_id: int,
    filename: str,
    text: str,
) -> int:
    chunks = chunk_text(text)
    if not chunks:
        return 0

    db.query(RagChunkRecord).filter(RagChunkRecord.document_id == document_id).delete()
    embeddings = embed_texts(chunks)

    for chunk_index, (chunk_text_value, embedding) in enumerate(zip(chunks, embeddings, strict=False)):
        db.add(
            RagChunkRecord(
                user_id=user_id,
                document_id=document_id,
                filename=filename,
                chunk_index=chunk_index,
                chunk_text=chunk_text_value,
                embedding_json=json.dumps(embedding),
            )
        )

    db.commit()
    return len(chunks)


def index_knowledge_base_chunks(
    db: Session,
    knowledge_base_id: int,
    title: str,
    source_url: str,
    text: str,
) -> int:
    chunks = chunk_text(text)
    if not chunks:
        return 0

    db.query(KnowledgeBaseChunkRecord).filter(
        KnowledgeBaseChunkRecord.knowledge_base_id == knowledge_base_id
    ).delete()
    embeddings = embed_texts(chunks)

    for chunk_index, (chunk_text_value, embedding) in enumerate(zip(chunks, embeddings, strict=False)):
        db.add(
            KnowledgeBaseChunkRecord(
                knowledge_base_id=knowledge_base_id,
                title=title,
                source_url=source_url,
                chunk_index=chunk_index,
                chunk_text=chunk_text_value,
                embedding_json=json.dumps(embedding),
            )
        )

    db.commit()
    return len(chunks)


def search_similar_chunks(
    db: Session,
    user_id: int,
    question: str,
    top_k: int | None = None,
    search_scope: str = "all",
) -> list[dict]:
    query_embedding = embed_texts([question])[0]
    scored: list[dict] = []

    if search_scope in {"user", "all"}:
        chunk_rows = db.query(RagChunkRecord).filter(RagChunkRecord.user_id == user_id).all()
        for row in chunk_rows:
            embedding = json.loads(row.embedding_json)
            score = cosine_similarity(query_embedding, embedding)
            scored.append(
                {
                    "source_kind": "user_document",
                    "document_id": row.document_id,
                    "knowledge_base_id": None,
                    "filename": row.filename,
                    "title": row.filename,
                    "source_url": None,
                    "chunk_text": row.chunk_text,
                    "score": score,
                }
            )

    if search_scope in {"official", "all"}:
        kb_rows = db.query(KnowledgeBaseChunkRecord).all()
        for row in kb_rows:
            embedding = json.loads(row.embedding_json)
            score = cosine_similarity(query_embedding, embedding)
            scored.append(
                {
                    "source_kind": "official_law",
                    "document_id": None,
                    "knowledge_base_id": row.knowledge_base_id,
                    "filename": None,
                    "title": row.title,
                    "source_url": row.source_url,
                    "chunk_text": row.chunk_text,
                    "score": score,
                }
            )

    scored.sort(key=lambda item: item["score"], reverse=True)
    return scored[: (top_k or settings.rag_top_k)]


def build_rag_context(chunks: list[dict]) -> str:
    if not chunks:
        return "В доступных источниках не найдено релевантных фрагментов."

    context_parts = ["Контекст из доступных источников:"]
    for index, chunk in enumerate(chunks, start=1):
        if chunk["source_kind"] == "official_law":
            label = f"Официальный акт #{chunk['knowledge_base_id']} ({chunk['title']})"
        else:
            label = f"Документ пользователя #{chunk['document_id']} ({chunk['filename']})"
        context_parts.append(f"[Источник {index}] {label}\n{chunk['chunk_text']}")
    return "\n\n".join(context_parts)
