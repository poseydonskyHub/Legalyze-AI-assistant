#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/backend"

echo "=== Legalyze backend: Linux запуск ==="

if ! command -v python >/dev/null 2>&1; then
  echo "Python не найден. Установите Python 3.11+ и попробуйте снова."
  exit 1
fi

if [ ! -x ".venv/bin/python" ]; then
  echo "Создаю виртуальное окружение..."
  python -m venv .venv
  echo "Устанавливаю зависимости..."
  .venv/bin/python -m pip install --upgrade pip
  .venv/bin/python -m pip install -r requirements.txt
fi

if [ ! -f ".env" ]; then
  cp .env.example .env
  echo "Файл .env создан из .env.example"
  echo "ВАЖНО: откройте backend/.env и укажите OPENAI_API_KEY"
fi

if grep -q '^OPENAI_API_KEY=your_openai_api_key_here$' .env; then
  echo "В файле backend/.env еще не указан реальный OPENAI_API_KEY."
  echo "Откройте файл backend/.env, вставьте ключ и запустите этот скрипт снова."
  exit 1
fi

echo "Запускаю backend на http://127.0.0.1:8000"
exec .venv/bin/python -m uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
