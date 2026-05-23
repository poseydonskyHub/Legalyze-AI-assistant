from __future__ import annotations

from dataclasses import dataclass
import logging
from urllib.parse import urljoin, urlparse

import requests
from bs4 import BeautifulSoup
from sqlalchemy.orm import Session

from app.config import get_settings
from app.db.models import KnowledgeBaseRecord
from app.services.document_service import parse_html, parse_pdf_text

ALLOWED_NETLOCS = {
    "pravo.gov.ru",
    "www.pravo.gov.ru",
    "publication.pravo.gov.ru",
    "www.publication.pravo.gov.ru",
}

logger = logging.getLogger(__name__)
settings = get_settings()


@dataclass
class IngestedLaw:
    title: str
    source_url: str
    filename: str | None
    content_type: str | None
    extension: str | None
    extracted_text: str


def validate_pravo_url(url: str) -> None:
    parsed = urlparse(url)
    if parsed.scheme not in {"http", "https"} or parsed.netloc.lower() not in ALLOWED_NETLOCS:
        raise ValueError("Разрешены только URL с доменов pravo.gov.ru и publication.pravo.gov.ru.")


def fetch_url(url: str) -> requests.Response:
    response = requests.get(url, timeout=30, headers={"User-Agent": "LegalAssistantBot/1.0"})
    response.raise_for_status()
    return response


def extract_title(soup: BeautifulSoup, fallback_url: str) -> str:
    if soup.title and soup.title.text.strip():
        return soup.title.text.strip()
    h1 = soup.find("h1")
    if h1 and h1.get_text(strip=True):
        return h1.get_text(strip=True)
    return fallback_url


def collect_pdf_links(base_url: str, soup: BeautifulSoup) -> list[tuple[str, str]]:
    links: list[tuple[str, str]] = []
    seen: set[str] = set()

    for anchor in soup.find_all("a", href=True):
        href = anchor["href"].strip()
        full_url = urljoin(base_url, href)
        if ".pdf" not in full_url.lower():
            continue
        if full_url in seen:
            continue
        seen.add(full_url)
        title = anchor.get_text(" ", strip=True) or full_url
        links.append((title, full_url))

    return links


def ingest_single_resource(url: str, title_hint: str | None = None) -> IngestedLaw:
    response = fetch_url(url)
    content_type = response.headers.get("Content-Type", "")
    lower_url = url.lower()

    if ".pdf" in lower_url or "application/pdf" in content_type.lower():
        extracted_text = parse_pdf_text(response.content)
        filename = lower_url.rsplit("/", 1)[-1] or None
        return IngestedLaw(
            title=title_hint or filename or url,
            source_url=url,
            filename=filename,
            content_type=content_type or "application/pdf",
            extension=".pdf",
            extracted_text=extracted_text,
        )

    html_text = parse_html(response.content)
    filename = lower_url.rsplit("/", 1)[-1] or None
    soup = BeautifulSoup(response.text, "html.parser")
    title = title_hint or extract_title(soup, url)
    return IngestedLaw(
        title=title,
        source_url=url,
        filename=filename,
        content_type=content_type or "text/html",
        extension=".html",
        extracted_text=html_text,
    )


def discover_resources(url: str, max_documents: int) -> list[tuple[str, str]]:
    response = fetch_url(url)
    content_type = response.headers.get("Content-Type", "")
    if ".pdf" in url.lower() or "application/pdf" in content_type.lower():
        return [(url.rsplit("/", 1)[-1] or url, url)]

    soup = BeautifulSoup(response.text, "html.parser")
    pdf_links = collect_pdf_links(url, soup)
    if pdf_links:
        return pdf_links[:max_documents]

    title = extract_title(soup, url)
    return [(title, url)]


def ingest_pravo_url(db: Session, url: str, max_documents: int) -> tuple[list[KnowledgeBaseRecord], list[str]]:
    validate_pravo_url(url)
    discovered = discover_resources(url, max_documents)
    ingested: list[KnowledgeBaseRecord] = []
    skipped: list[str] = []

    for title_hint, resource_url in discovered:
        existing = db.query(KnowledgeBaseRecord).filter(KnowledgeBaseRecord.source_url == resource_url).first()
        if existing:
            skipped.append(resource_url)
            continue

        law = ingest_single_resource(resource_url, title_hint=title_hint)
        record = KnowledgeBaseRecord(
            title=law.title[:500],
            source_url=law.source_url,
            source_type="official_law",
            filename=law.filename,
            extension=law.extension,
            content_type=law.content_type,
            extracted_text=law.extracted_text,
            indexed_in_rag="no",
        )
        db.add(record)
        db.commit()
        db.refresh(record)
        ingested.append(record)

    return ingested, skipped


def seed_default_knowledge_base(db: Session) -> tuple[int, int]:
    from app.rag.service import index_knowledge_base_chunks

    ingested_total = 0
    indexed_total = 0

    for url in settings.default_pravo_urls:
        try:
            ingested, _skipped = ingest_pravo_url(db, url, settings.default_pravo_max_documents)
        except Exception as exc:
            logger.warning("Failed to ingest default pravo.gov.ru source %s: %s", url, exc)
            continue

        ingested_total += len(ingested)

        for record in ingested:
            try:
                indexed_count = index_knowledge_base_chunks(
                    db=db,
                    knowledge_base_id=record.id,
                    title=record.title,
                    source_url=record.source_url,
                    text=record.extracted_text,
                )
                record.indexed_in_rag = "yes" if indexed_count > 0 else "no"
                indexed_total += indexed_count
            except Exception as exc:
                logger.warning("Failed to index default law %s: %s", record.source_url, exc)
                record.indexed_in_rag = "error"
            db.add(record)
            db.commit()

    return ingested_total, indexed_total
