from __future__ import annotations

import base64
import json
import uuid
from datetime import datetime, timedelta, timezone

import requests
from fastapi import HTTPException
from sqlalchemy.orm import Session

from app.config import get_settings
from app.db.models import PaymentRecord, User

settings = get_settings()

PAYMENT_PLANS = {
    "pro_month": {
        "code": "pro_month",
        "title": "Pro на 30 дней",
        "price_value": "990.00",
        "currency": "RUB",
        "duration_days": 30,
        "features": [
            "RAG по пользовательским документам",
            "RAG по официальным актам",
            "Streaming ответы",
            "Приоритетные лимиты",
        ],
    },
    "pro_year": {
        "code": "pro_year",
        "title": "Pro на 365 дней",
        "price_value": "9990.00",
        "currency": "RUB",
        "duration_days": 365,
        "features": [
            "RAG по пользовательским документам",
            "RAG по официальным актам",
            "Streaming ответы",
            "Экономия на годовой подписке",
        ],
    },
}


def get_payment_plans() -> list[dict]:
    return list(PAYMENT_PLANS.values())


def get_plan_or_raise(plan_code: str) -> dict:
    plan = PAYMENT_PLANS.get(plan_code)
    if not plan:
        raise HTTPException(status_code=400, detail="Неизвестный тариф.")
    return plan


def get_yookassa_auth_header() -> str:
    if not settings.yookassa_shop_id or not settings.yookassa_secret_key:
        raise HTTPException(
            status_code=500,
            detail="YOOKASSA_SHOP_ID или YOOKASSA_SECRET_KEY не заданы в .env.",
        )
    raw = f"{settings.yookassa_shop_id}:{settings.yookassa_secret_key}".encode("utf-8")
    return "Basic " + base64.b64encode(raw).decode("utf-8")


def create_yookassa_payment(db: Session, user: User, plan_code: str) -> PaymentRecord:
    plan = get_plan_or_raise(plan_code)
    idempotence_key = str(uuid.uuid4())

    payload = {
        "amount": {
            "value": plan["price_value"],
            "currency": plan["currency"],
        },
        "capture": True,
        "confirmation": {
            "type": "redirect",
            "return_url": settings.yookassa_return_url,
        },
        "description": f"{plan['title']} для пользователя #{user.id}",
        "metadata": {
            "user_id": str(user.id),
            "plan_code": plan["code"],
        },
    }

    response = requests.post(
        "https://api.yookassa.ru/v3/payments",
        headers={
            "Authorization": get_yookassa_auth_header(),
            "Idempotence-Key": idempotence_key,
            "Content-Type": "application/json",
        },
        json=payload,
        timeout=30,
    )
    response.raise_for_status()
    data = response.json()

    payment = PaymentRecord(
        user_id=user.id,
        provider="yookassa",
        plan_code=plan["code"],
        amount_value=plan["price_value"],
        amount_currency=plan["currency"],
        status=data.get("status", "pending"),
        yookassa_payment_id=data["id"],
        idempotence_key=idempotence_key,
        confirmation_url=data.get("confirmation", {}).get("confirmation_url"),
        description=payload["description"],
        raw_payload=json.dumps(data, ensure_ascii=False),
    )
    db.add(payment)
    db.commit()
    db.refresh(payment)
    return payment


def get_payment_by_id(db: Session, payment_id: int, user: User) -> PaymentRecord:
    payment = db.query(PaymentRecord).filter(PaymentRecord.id == payment_id, PaymentRecord.user_id == user.id).first()
    if not payment:
        raise HTTPException(status_code=404, detail="Платёж не найден.")
    return payment


def activate_subscription_for_payment(db: Session, payment: PaymentRecord, user: User) -> None:
    plan = get_plan_or_raise(payment.plan_code)
    expires_at = datetime.now(timezone.utc) + timedelta(days=plan["duration_days"])
    user.subscription_plan = plan["code"]
    user.subscription_status = "active"
    user.subscription_expires_at = expires_at.isoformat()
    db.add(user)
    db.commit()


def process_yookassa_webhook(db: Session, body: dict) -> dict:
    event = body.get("event")
    obj = body.get("object", {})
    yookassa_payment_id = obj.get("id")
    if not yookassa_payment_id:
        raise HTTPException(status_code=400, detail="В webhook отсутствует payment id.")

    payment = db.query(PaymentRecord).filter(PaymentRecord.yookassa_payment_id == yookassa_payment_id).first()
    if not payment:
        return {"status": "ignored", "detail": "Платёж не найден локально."}

    payment.status = obj.get("status", payment.status)
    payment.raw_payload = json.dumps(body, ensure_ascii=False)
    db.add(payment)
    db.commit()
    db.refresh(payment)

    if event == "payment.succeeded":
        user = db.query(User).filter(User.id == payment.user_id).first()
        if user:
            activate_subscription_for_payment(db, payment, user)

    return {"status": "ok", "detail": event or "processed"}
