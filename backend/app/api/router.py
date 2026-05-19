from fastapi import APIRouter

from app.api.routes_auth import router as auth_router
from app.api.routes_chat import router as chat_router
from app.api.routes_files import router as files_router
from app.api.routes_state import router as state_router
from app.api.routes_system import router as system_router

api_router = APIRouter()
api_router.include_router(system_router, tags=["system"])
api_router.include_router(auth_router, tags=["auth"])
api_router.include_router(chat_router, tags=["chat"])
api_router.include_router(files_router, tags=["files"])
api_router.include_router(state_router, tags=["stateful"])
