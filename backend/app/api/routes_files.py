from fastapi import APIRouter, File, Form, HTTPException, UploadFile

from app.models.schemas import AnalyzeDocumentResponse, ParsedFileResponse
from app.services.ai_service import analyze_text, settings
from app.services.document_service import ensure_size_limit, parse_upload_file

router = APIRouter()


@router.post("/parse-file", response_model=ParsedFileResponse)
async def parse_file(file: UploadFile = File(...)) -> ParsedFileResponse:
    file_bytes = await file.read()
    ensure_size_limit(file_bytes)

    try:
        extracted_text, extracted_via, extension = parse_upload_file(
            filename=file.filename or "",
            content_type=file.content_type,
            file_bytes=file_bytes,
        )
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=400, detail=f"Не удалось обработать файл: {exc}") from exc

    if not extracted_text.strip():
        raise HTTPException(
            status_code=422,
            detail="Не удалось извлечь текст из файла. Возможно, файл пустой или поврежден.",
        )

    return ParsedFileResponse(
        filename=file.filename or "unknown",
        extension=extension,
        extracted_via=extracted_via,
        extracted_text=extracted_text,
        chars_count=len(extracted_text),
    )


@router.post("/analyze-upload", response_model=AnalyzeDocumentResponse)
async def analyze_upload(
    file: UploadFile = File(...),
    analysis_type: str = Form("general"),
    question: str | None = Form(None),
) -> AnalyzeDocumentResponse:
    file_bytes = await file.read()
    ensure_size_limit(file_bytes)

    try:
        extracted_text, _, _ = parse_upload_file(
            filename=file.filename or "",
            content_type=file.content_type,
            file_bytes=file_bytes,
        )
        summary = analyze_text(
            analysis_type=analysis_type,
            document_text=extracted_text,
            question=question,
            filename=file.filename,
        )
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка обработки файла: {exc}") from exc

    return AnalyzeDocumentResponse(
        summary=summary,
        model=settings.openai_model,
        disclaimer=(
            "Анализ является предварительным и требует проверки юристом перед "
            "использованием в реальной правовой ситуации."
        ),
    )
