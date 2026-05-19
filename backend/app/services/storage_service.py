from __future__ import annotations

import os
import uuid
from pathlib import Path

from app.config import get_settings

settings = get_settings()


def ensure_upload_dir() -> Path:
    upload_dir = Path(settings.upload_dir)
    upload_dir.mkdir(parents=True, exist_ok=True)
    return upload_dir


def save_uploaded_file(filename: str, file_bytes: bytes) -> str:
    upload_dir = ensure_upload_dir()
    safe_name = Path(filename).name
    stored_name = f"{uuid.uuid4().hex}_{safe_name}"
    path = upload_dir / stored_name
    path.write_bytes(file_bytes)
    return os.fspath(path.resolve())
