from datetime import datetime

from sqlalchemy import DateTime, ForeignKey, Integer, String, Text, func
from sqlalchemy.orm import Mapped, mapped_column, relationship

from app.db.session import Base


class User(Base):
    __tablename__ = "users"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    email: Mapped[str] = mapped_column(String(255), unique=True, index=True)
    full_name: Mapped[str] = mapped_column(String(255))
    password_hash: Mapped[str] = mapped_column(String(255))
    api_token: Mapped[str | None] = mapped_column(String(255), unique=True, index=True, nullable=True)
    device_id: Mapped[str | None] = mapped_column(String(255), unique=True, index=True, nullable=True)
    subscription_plan: Mapped[str] = mapped_column(String(50), default="free")
    subscription_status: Mapped[str] = mapped_column(String(50), default="inactive")
    subscription_expires_at: Mapped[str | None] = mapped_column(String(100), nullable=True)
    daily_request_count: Mapped[int] = mapped_column(Integer, default=0)
    daily_request_date: Mapped[str | None] = mapped_column(String(20), nullable=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    conversations: Mapped[list["Conversation"]] = relationship(back_populates="user", cascade="all, delete-orphan")
    documents: Mapped[list["DocumentRecord"]] = relationship(back_populates="user", cascade="all, delete-orphan")


class DeviceUsageRecord(Base):
    __tablename__ = "device_usage"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    device_id: Mapped[str] = mapped_column(String(255), unique=True, index=True)
    daily_request_count: Mapped[int] = mapped_column(Integer, default=0)
    daily_request_date: Mapped[str | None] = mapped_column(String(20), nullable=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
    )


class Conversation(Base):
    __tablename__ = "conversations"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"), index=True)
    title: Mapped[str] = mapped_column(String(255), default="Новый диалог")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
    )

    user: Mapped["User"] = relationship(back_populates="conversations")
    messages: Mapped[list["Message"]] = relationship(back_populates="conversation", cascade="all, delete-orphan")


class Message(Base):
    __tablename__ = "messages"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    conversation_id: Mapped[int] = mapped_column(ForeignKey("conversations.id"), index=True)
    role: Mapped[str] = mapped_column(String(50))
    content: Mapped[str] = mapped_column(Text())
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    conversation: Mapped["Conversation"] = relationship(back_populates="messages")


class DocumentRecord(Base):
    __tablename__ = "documents"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"), index=True)
    filename: Mapped[str] = mapped_column(String(255))
    extension: Mapped[str] = mapped_column(String(20))
    content_type: Mapped[str | None] = mapped_column(String(100), nullable=True)
    storage_path: Mapped[str] = mapped_column(String(500))
    extracted_via: Mapped[str] = mapped_column(String(50))
    extracted_text: Mapped[str] = mapped_column(Text())
    analysis_summary: Mapped[str | None] = mapped_column(Text(), nullable=True)
    indexed_in_rag: Mapped[str] = mapped_column(String(10), default="no")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())

    user: Mapped["User"] = relationship(back_populates="documents")


class RagChunkRecord(Base):
    __tablename__ = "rag_chunks"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"), index=True)
    document_id: Mapped[int] = mapped_column(ForeignKey("documents.id"), index=True)
    filename: Mapped[str] = mapped_column(String(255))
    chunk_index: Mapped[int] = mapped_column(index=True)
    chunk_text: Mapped[str] = mapped_column(Text())
    embedding_json: Mapped[str] = mapped_column(Text())
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())


class KnowledgeBaseRecord(Base):
    __tablename__ = "knowledge_base"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    source_url: Mapped[str] = mapped_column(String(1000), unique=True, index=True)
    title: Mapped[str] = mapped_column(String(500))
    source_type: Mapped[str] = mapped_column(String(50), default="official_law")
    filename: Mapped[str | None] = mapped_column(String(255), nullable=True)
    extension: Mapped[str | None] = mapped_column(String(20), nullable=True)
    content_type: Mapped[str | None] = mapped_column(String(100), nullable=True)
    extracted_text: Mapped[str] = mapped_column(Text())
    indexed_in_rag: Mapped[str] = mapped_column(String(10), default="no")
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())


class KnowledgeBaseChunkRecord(Base):
    __tablename__ = "knowledge_base_chunks"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    knowledge_base_id: Mapped[int] = mapped_column(ForeignKey("knowledge_base.id"), index=True)
    title: Mapped[str] = mapped_column(String(500))
    source_url: Mapped[str] = mapped_column(String(1000))
    chunk_index: Mapped[int] = mapped_column(index=True)
    chunk_text: Mapped[str] = mapped_column(Text())
    embedding_json: Mapped[str] = mapped_column(Text())
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())


class PaymentRecord(Base):
    __tablename__ = "payments"

    id: Mapped[int] = mapped_column(primary_key=True, index=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"), index=True)
    provider: Mapped[str] = mapped_column(String(50), default="yookassa")
    plan_code: Mapped[str] = mapped_column(String(50))
    amount_value: Mapped[str] = mapped_column(String(50))
    amount_currency: Mapped[str] = mapped_column(String(10), default="RUB")
    status: Mapped[str] = mapped_column(String(50), default="pending")
    yookassa_payment_id: Mapped[str | None] = mapped_column(String(255), unique=True, nullable=True)
    idempotence_key: Mapped[str] = mapped_column(String(255), unique=True)
    confirmation_url: Mapped[str | None] = mapped_column(String(1000), nullable=True)
    description: Mapped[str] = mapped_column(String(500))
    raw_payload: Mapped[str | None] = mapped_column(Text(), nullable=True)
    created_at: Mapped[datetime] = mapped_column(DateTime(timezone=True), server_default=func.now())
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
    )
