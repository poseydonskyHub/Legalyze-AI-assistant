from pathlib import Path

from fastapi import APIRouter
from fastapi.responses import FileResponse

from app.config import get_settings
from app.models.schemas import AppInfoResponse

router = APIRouter()
settings = get_settings()
SITE_INDEX = Path(__file__).resolve().parents[2] / "site" / "index.html"


@router.get("/")
def root() -> dict[str, str]:
    return {
        "message": "AI Legal Assistant API is running",
        "docs": "/docs",
        "landing": "/landing",
    }


@router.get("/health")
def healthcheck() -> dict[str, str]:
    return {"status": "ok", "app": settings.app_name}


@router.get("/ready")
def readiness() -> dict[str, str]:
    return {"status": "ready", "version": settings.api_version}


@router.get("/app-info", response_model=AppInfoResponse)
def app_info() -> AppInfoResponse:
    return AppInfoResponse(
        app_name=settings.app_name,
        api_version=settings.api_version,
        openai_model=settings.openai_model,
        embedding_model=settings.embedding_model,
        supports_chat_stream=True,
        supports_rag_stream=True,
        supports_knowledge_base=True,
        supports_payments=False,
        max_file_size_bytes=settings.max_file_size_bytes,
        demo_daily_request_limit=settings.demo_daily_request_limit,
        donation_url=settings.donation_url,
    )


@router.get("/landing")
def landing_page() -> FileResponse:
    return FileResponse(SITE_INDEX)
