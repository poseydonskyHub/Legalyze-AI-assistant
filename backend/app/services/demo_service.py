from datetime import date

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app.config import get_settings
from app.db.models import DeviceUsageRecord, User
from app.models.schemas import UsageResponse

settings = get_settings()


def _today_iso() -> str:
    return date.today().isoformat()


def _ensure_device_id(user: User) -> str:
    device_id = (user.device_id or "").strip()
    if not device_id:
        raise HTTPException(
            status_code=400,
            detail="У текущего аккаунта нет привязки к устройству. Войдите в аккаунт заново из приложения.",
        )
    return device_id


def _get_or_create_device_usage(db: Session, device_id: str) -> DeviceUsageRecord:
    record = db.query(DeviceUsageRecord).filter(DeviceUsageRecord.device_id == device_id).first()
    if record is None:
        record = DeviceUsageRecord(
            device_id=device_id,
            daily_request_count=0,
            daily_request_date=_today_iso(),
        )
        db.add(record)
        db.commit()
        db.refresh(record)
    return record


def sync_daily_usage(db: Session, user: User) -> DeviceUsageRecord:
    device_id = _ensure_device_id(user)
    record = _get_or_create_device_usage(db, device_id)
    today = _today_iso()
    if record.daily_request_date != today:
        record.daily_request_date = today
        record.daily_request_count = 0
        db.add(record)
        db.commit()
        db.refresh(record)
    return record


def get_usage_status(db: Session, user: User) -> UsageResponse:
    record = sync_daily_usage(db, user)
    used_today = int(record.daily_request_count or 0)
    remaining = max(settings.demo_daily_request_limit - used_today, 0)
    return UsageResponse(
        daily_limit=settings.demo_daily_request_limit,
        used_today=used_today,
        remaining_today=remaining,
        reset_date=record.daily_request_date or _today_iso(),
        subscription_plan="demo",
        subscription_status="active",
    )


def consume_demo_request(db: Session, user: User) -> UsageResponse:
    record = sync_daily_usage(db, user)
    used_today = int(record.daily_request_count or 0)
    if used_today >= settings.demo_daily_request_limit:
        raise HTTPException(
            status_code=429,
            detail=(
                f"Дневной лимит демо-версии для этого устройства исчерпан: {settings.demo_daily_request_limit} запросов в день. "
                "Лимит обновится автоматически завтра."
            ),
        )

    record.daily_request_count = used_today + 1
    db.add(record)
    db.commit()
    db.refresh(record)
    return get_usage_status(db, user)
