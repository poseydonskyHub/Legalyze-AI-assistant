from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session

from app.db.models import User
from app.db.session import get_db
from app.models.schemas import AuthResponse, DeleteResponse, UserLoginRequest, UserRegisterRequest, UserResponse
from app.services.auth_service import generate_token, get_current_user, hash_password, verify_password

router = APIRouter(prefix="/auth")


def _normalize_device_id(device_id: str) -> str:
    return device_id.strip()


@router.post("/register", response_model=AuthResponse)
def register(payload: UserRegisterRequest, db: Session = Depends(get_db)) -> AuthResponse:
    existing = db.query(User).filter(User.email == payload.email.lower().strip()).first()
    if existing:
        raise HTTPException(status_code=409, detail="Пользователь с таким email уже существует.")

    normalized_device_id = _normalize_device_id(payload.device_id)
    device_owner = db.query(User).filter(User.device_id == normalized_device_id).first()
    if device_owner:
        raise HTTPException(
            status_code=409,
            detail="На этой установке приложения уже зарегистрирован демо-аккаунт. Используйте существующий аккаунт.",
        )

    user = User(
        email=payload.email.lower().strip(),
        full_name=payload.full_name.strip(),
        password_hash=hash_password(payload.password),
        api_token=generate_token(),
        device_id=normalized_device_id,
    )
    db.add(user)
    db.commit()
    db.refresh(user)

    return AuthResponse(
        access_token=user.api_token or "",
        user_id=user.id,
        email=user.email,
        full_name=user.full_name,
    )


@router.post("/login", response_model=AuthResponse)
def login(payload: UserLoginRequest, db: Session = Depends(get_db)) -> AuthResponse:
    user = db.query(User).filter(User.email == payload.email.lower().strip()).first()
    if not user or not verify_password(payload.password, user.password_hash):
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Неверный email или пароль.")

    normalized_device_id = _normalize_device_id(payload.device_id)
    if not user.device_id:
        device_owner = db.query(User).filter(User.device_id == normalized_device_id, User.id != user.id).first()
        if device_owner:
            raise HTTPException(
                status_code=409,
                detail="Эта установка уже привязана к другому демо-аккаунту. Войдите в ранее созданный аккаунт.",
            )
        user.device_id = normalized_device_id
        db.add(user)
        db.commit()
        db.refresh(user)

    if not user.api_token:
        user.api_token = generate_token()
        db.add(user)
        db.commit()
        db.refresh(user)

    return AuthResponse(
        access_token=user.api_token,
        user_id=user.id,
        email=user.email,
        full_name=user.full_name,
    )


@router.post("/logout", response_model=DeleteResponse)
def logout(
    current_user: User = Depends(get_current_user),
    db: Session = Depends(get_db),
) -> DeleteResponse:
    current_user.api_token = None
    db.add(current_user)
    db.commit()
    return DeleteResponse(status="ok", detail="Выход выполнен.")


@router.get("/me", response_model=UserResponse)
def me(current_user: User = Depends(get_current_user)) -> UserResponse:
    return UserResponse(
        id=current_user.id,
        email=current_user.email,
        full_name=current_user.full_name,
        subscription_plan="demo",
        subscription_status="active",
        subscription_expires_at=None,
    )
