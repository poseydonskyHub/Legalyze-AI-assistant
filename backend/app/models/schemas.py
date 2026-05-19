from typing import Literal

from pydantic import BaseModel, Field


class UserRegisterRequest(BaseModel):
    email: str = Field(..., min_length=5)
    full_name: str = Field(..., min_length=2)
    password: str = Field(..., min_length=6)
    device_id: str = Field(..., min_length=8)


class UserLoginRequest(BaseModel):
    email: str
    password: str
    device_id: str = Field(..., min_length=8)


class AuthResponse(BaseModel):
    access_token: str
    token_type: str = "bearer"
    user_id: int
    email: str
    full_name: str


class UserResponse(BaseModel):
    id: int
    email: str
    full_name: str
    subscription_plan: str = "free"
    subscription_status: str = "inactive"
    subscription_expires_at: str | None = None


class DeleteResponse(BaseModel):
    status: str
    detail: str


class UsageResponse(BaseModel):
    daily_limit: int
    used_today: int
    remaining_today: int
    reset_date: str
    subscription_plan: str = "demo"
    subscription_status: str = "active"


class AppInfoResponse(BaseModel):
    app_name: str
    api_version: str
    openai_model: str
    embedding_model: str
    supports_chat_stream: bool
    supports_rag_stream: bool
    supports_knowledge_base: bool
    supports_payments: bool
    max_file_size_bytes: int
    demo_daily_request_limit: int
    donation_url: str | None = None


class PaymentPlanResponse(BaseModel):
    code: str
    title: str
    price_value: str
    currency: str
    duration_days: int
    features: list[str]


class CreatePaymentRequest(BaseModel):
    plan_code: str


class CreatePaymentResponse(BaseModel):
    payment_id: int
    provider: str
    status: str
    confirmation_url: str
    yookassa_payment_id: str
    amount_value: str
    amount_currency: str


class PaymentStatusResponse(BaseModel):
    payment_id: int
    provider: str
    status: str
    plan_code: str
    amount_value: str
    amount_currency: str
    confirmation_url: str | None = None


class ChatRequest(BaseModel):
    message: str = Field(..., min_length=3, description="Вопрос пользователя")
    context: str | None = None
    conversation_id: str | None = None


class ChatResponse(BaseModel):
    answer: str
    model: str
    conversation_id: str | None = None
    disclaimer: str


class StoredChatResponse(ChatResponse):
    conversation_id: int


class StoredChatRequest(BaseModel):
    message: str = Field(..., min_length=3)
    context: str | None = None
    conversation_id: int | None = None
    title: str | None = None


class AnalyzeDocumentRequest(BaseModel):
    document_text: str = Field(..., min_length=20)
    analysis_type: Literal["contract", "claim", "lawsuit", "general"] = "general"
    question: str | None = None


class AnalyzeDocumentResponse(BaseModel):
    summary: str
    model: str
    disclaimer: str


class ParsedFileResponse(BaseModel):
    filename: str
    extension: str
    extracted_via: str
    extracted_text: str
    chars_count: int


class ConversationCreateRequest(BaseModel):
    title: str = Field(..., min_length=1, max_length=255)


class ConversationUpdateRequest(BaseModel):
    title: str = Field(..., min_length=1, max_length=255)


class ConversationResponse(BaseModel):
    id: int
    title: str
    created_at: str
    updated_at: str


class MessageResponse(BaseModel):
    id: int
    role: str
    content: str
    created_at: str


class DocumentResponse(BaseModel):
    id: int
    filename: str
    extension: str
    content_type: str | None
    extracted_via: str
    extracted_text: str
    analysis_summary: str | None = None
    indexed_in_rag: str
    created_at: str


class KnowledgeBaseIngestRequest(BaseModel):
    url: str
    max_documents: int = Field(default=5, ge=1, le=20)


class KnowledgeBaseResponse(BaseModel):
    id: int
    title: str
    source_url: str
    source_type: str
    filename: str | None
    indexed_in_rag: str
    created_at: str


class KnowledgeBaseIngestResponse(BaseModel):
    ingested: list[KnowledgeBaseResponse]
    skipped: list[str]


class RagQueryRequest(BaseModel):
    question: str = Field(..., min_length=3)
    conversation_id: int | None = None
    top_k: int | None = None
    title: str | None = None
    search_scope: Literal["user", "official", "all"] = "all"


class RagSource(BaseModel):
    source_kind: Literal["user_document", "official_law"]
    document_id: int | None = None
    knowledge_base_id: int | None = None
    filename: str | None = None
    title: str | None = None
    source_url: str | None = None
    chunk_text: str
    score: float


class RagCitation(BaseModel):
    title: str
    quote: str
    article_label: str | None = None
    article_url: str | None = None
    document_id: int | None = None
    knowledge_base_id: int | None = None
    filename: str | None = None
    source_url: str | None = None


class RagStructuredAnswer(BaseModel):
    answer: str
    citations: list[RagCitation]
    confidence_score: float
    refusal_reason: str | None = None
    source_validation: str


class RagQueryResponse(BaseModel):
    answer: str
    model: str
    conversation_id: int
    sources: list[RagSource]
    citations: list[RagCitation]
    confidence_score: float
    refusal_reason: str | None = None
    source_validation: str
    disclaimer: str
