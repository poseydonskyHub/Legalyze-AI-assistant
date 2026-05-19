from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.api.router import api_router
from app.config import get_settings
from app.db.base import init_db

settings = get_settings()

app = FastAPI(
    title=settings.app_name,
    version=settings.api_version,
    description="Backend API для ИИ-юриста ассистента.",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_allow_origins or ["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(api_router)


@app.on_event("startup")
def on_startup() -> None:
    init_db()
