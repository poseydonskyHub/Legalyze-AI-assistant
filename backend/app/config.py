from functools import lru_cache
import os

from dotenv import load_dotenv
from pydantic import BaseModel

load_dotenv()

def normalize_database_url(raw_url: str) -> str:
    url = raw_url.strip()
    if url.startswith("postgresql://") and "+psycopg" not in url:
        return "postgresql+psycopg://" + url.removeprefix("postgresql://")
    if url.startswith("postgres://"):
        return "postgresql+psycopg://" + url.removeprefix("postgres://")
    return url

class Settings(BaseModel):
    app_name: str = os.getenv("APP_NAME", "AI Legal Assistant API")
    api_version: str = os.getenv("API_VERSION", "1.4.0")
    openai_api_key: str | None = os.getenv("OPENAI_API_KEY")
    openai_model: str = os.getenv("OPENAI_MODEL", "gpt-4.1")
    max_file_size_bytes: int = int(os.getenv("MAX_FILE_SIZE_BYTES", 15728640))
    database_url: str = normalize_database_url(
        os.getenv("DATABASE_URL", "sqlite:///./legal_assistant.db")
    )
    upload_dir: str = os.getenv("UPLOAD_DIR", "./storage/uploads")
    embedding_model: str = os.getenv("EMBEDDING_MODEL", "text-embedding-3-large")
    embedding_dimensions: int = int(os.getenv("EMBEDDING_DIMENSIONS", 1536))
    rag_top_k: int = int(os.getenv("RAG_TOP_K", 5))
    rag_chunk_size: int = int(os.getenv("RAG_CHUNK_SIZE", 1200))
    rag_chunk_overlap: int = int(os.getenv("RAG_CHUNK_OVERLAP", 200))
    cors_allow_origins: list[str] = [
        origin.strip()
        for origin in os.getenv(
            "CORS_ALLOW_ORIGINS",
            "http://127.0.0.1,http://localhost,https://babies-olympic-acdbentity-coffee.trycloudflare.com",
        ).split(",")
        if origin.strip()
    ]
    demo_daily_request_limit: int = int(os.getenv("DEMO_DAILY_REQUEST_LIMIT", 5))
    donation_url: str = os.getenv("DONATION_URL", "https://dalink.to/legalyze")
    donation_widget_url: str = os.getenv(
        "DONATION_WIDGET_URL",
        "https://www.donationalerts.com/widget/goal/9634016?token=TpCZoV1v6U7yj9LhPKWE",
    )


@lru_cache
def get_settings() -> Settings:
    return Settings()
