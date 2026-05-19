from sqlalchemy import inspect, text

from app.db.session import Base, engine


def init_db() -> None:
    Base.metadata.create_all(bind=engine)
    ensure_schema_updates()


def ensure_schema_updates() -> None:
    inspector = inspect(engine)
    table_names = set(inspector.get_table_names())

    with engine.begin() as connection:
        if "device_usage" not in table_names:
            Base.metadata.tables["device_usage"].create(bind=connection, checkfirst=True)

        if "documents" in table_names:
            columns = {column["name"] for column in inspector.get_columns("documents")}
            if "analysis_summary" not in columns:
                connection.execute(text("ALTER TABLE documents ADD COLUMN analysis_summary TEXT"))
            if "indexed_in_rag" not in columns:
                connection.execute(text("ALTER TABLE documents ADD COLUMN indexed_in_rag VARCHAR(10) DEFAULT 'no'"))

        if "users" in table_names:
            columns = {column["name"] for column in inspector.get_columns("users")}
            indexes = {index["name"] for index in inspector.get_indexes("users")}
            if "subscription_plan" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN subscription_plan VARCHAR(50) DEFAULT 'free'"))
            if "subscription_status" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN subscription_status VARCHAR(50) DEFAULT 'inactive'"))
            if "subscription_expires_at" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN subscription_expires_at VARCHAR(100)"))
            if "device_id" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN device_id VARCHAR(255)"))
            if "daily_request_count" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN daily_request_count INTEGER DEFAULT 0"))
            if "daily_request_date" not in columns:
                connection.execute(text("ALTER TABLE users ADD COLUMN daily_request_date VARCHAR(20)"))
            if "ix_users_device_id" not in indexes:
                connection.execute(text("CREATE UNIQUE INDEX IF NOT EXISTS ix_users_device_id ON users (device_id)"))


def get_db_status() -> dict[str, str]:
    return {
        "status": "ready",
        "message": "База данных инициализирована.",
    }
