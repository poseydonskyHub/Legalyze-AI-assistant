from fastapi import APIRouter, HTTPException

from app.models.schemas import (
    AnalyzeDocumentRequest,
    AnalyzeDocumentResponse,
    ChatRequest,
    ChatResponse,
)
from app.services.ai_service import analyze_text, ask_legal_assistant, settings

router = APIRouter()


@router.post("/chat", response_model=ChatResponse)
def chat(payload: ChatRequest) -> ChatResponse:
    try:
        return ask_legal_assistant(payload)
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка LLM API: {exc}") from exc


@router.post("/analyze-document", response_model=AnalyzeDocumentResponse)
def analyze_document(payload: AnalyzeDocumentRequest) -> AnalyzeDocumentResponse:
    try:
        summary = analyze_text(
            analysis_type=payload.analysis_type,
            document_text=payload.document_text,
            question=payload.question,
        )
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка LLM API: {exc}") from exc

    return AnalyzeDocumentResponse(
        summary=summary,
        model=settings.openai_model,
        disclaimer=(
            "Анализ является предварительным и требует проверки юристом перед "
            "использованием в реальной правовой ситуации."
        ),
    )
