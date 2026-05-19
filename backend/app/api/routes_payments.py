from fastapi import APIRouter, Depends, HTTPException, Request
from sqlalchemy.orm import Session

from app.db.models import User
from app.db.session import get_db
from app.models.schemas import (
    CreatePaymentRequest,
    CreatePaymentResponse,
    DeleteResponse,
    PaymentPlanResponse,
    PaymentStatusResponse,
)
from app.services.auth_service import get_current_user
from app.services.payment_service import (
    create_yookassa_payment,
    get_payment_by_id,
    get_payment_plans,
    process_yookassa_webhook,
)

router = APIRouter(prefix="/payments")


@router.get("/plans", response_model=list[PaymentPlanResponse])
def list_plans() -> list[PaymentPlanResponse]:
    return [PaymentPlanResponse(**plan) for plan in get_payment_plans()]


@router.post("/create", response_model=CreatePaymentResponse)
def create_payment(
    payload: CreatePaymentRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> CreatePaymentResponse:
    try:
        payment = create_yookassa_payment(db, current_user, payload.plan_code)
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=502, detail=f"Ошибка создания платежа ЮKassa: {exc}") from exc

    return CreatePaymentResponse(
        payment_id=payment.id,
        provider=payment.provider,
        status=payment.status,
        confirmation_url=payment.confirmation_url or "",
        yookassa_payment_id=payment.yookassa_payment_id or "",
        amount_value=payment.amount_value,
        amount_currency=payment.amount_currency,
    )


@router.get("/{payment_id}", response_model=PaymentStatusResponse)
def get_payment_status(
    payment_id: int,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> PaymentStatusResponse:
    payment = get_payment_by_id(db, payment_id, current_user)
    return PaymentStatusResponse(
        payment_id=payment.id,
        provider=payment.provider,
        status=payment.status,
        plan_code=payment.plan_code,
        amount_value=payment.amount_value,
        amount_currency=payment.amount_currency,
        confirmation_url=payment.confirmation_url,
    )


@router.post("/webhook/yookassa", response_model=DeleteResponse)
async def yookassa_webhook(
    request: Request,
    db: Session = Depends(get_db),
) -> DeleteResponse:
    body = await request.json()
    result = process_yookassa_webhook(db, body)
    return DeleteResponse(status=result["status"], detail=result["detail"])
