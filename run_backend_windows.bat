@echo off
setlocal
cd /d "%~dp0backend"

echo === Legalyze backend: Windows запуск ===

where python >nul 2>nul
if errorlevel 1 (
    echo Python не найден. Установите Python 3.11+ и попробуйте снова.
    pause
    exit /b 1
)

if not exist ".venv\Scripts\python.exe" (
    echo Создаю виртуальное окружение...
    python -m venv .venv
    if errorlevel 1 (
        echo Не удалось создать виртуальное окружение.
        pause
        exit /b 1
    )
    echo Устанавливаю зависимости...
    .venv\Scripts\python.exe -m pip install --upgrade pip
    .venv\Scripts\python.exe -m pip install -r requirements.txt
)

if not exist ".env" (
    copy .env.example .env >nul
    echo Файл .env создан из .env.example
    echo ВАЖНО: откройте backend\.env и укажите OPENAI_API_KEY
)

findstr /B /C:"OPENAI_API_KEY=your_openai_api_key_here" .env >nul
if not errorlevel 1 (
    echo В файле backend\.env еще не указан реальный OPENAI_API_KEY.
    echo Откройте файл backend\.env, вставьте ключ и запустите этот файл снова.
    pause
    exit /b 1
)

echo Запускаю backend на http://127.0.0.1:8000
.venv\Scripts\python.exe -m uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload

pause
