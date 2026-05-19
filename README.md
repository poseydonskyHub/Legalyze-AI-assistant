# Legalyze

`Legalyze` — это desktop AI-ассистент для первичного юридического анализа документов.

Правильная модель использования для релиза такая:
- **ты** поднимаешь один backend на `Railway`
- **все пользователи** скачивают только desktop-клиент
- клиент автоматически подключается к твоему серверу

То есть обычным пользователям **не нужно запускать Python, FastAPI или backend локально**.

## Что входит в проект

- `backend/` — FastAPI сервер
- `qt_client/` — клиент на Qt/C++

## Что уже умеет приложение

- вход и регистрация
- сохранение сессии
- чат
- загрузка документов
- OCR и парсинг
- RAG по документам
- база официальных актов
- цитаты и ссылки на источники
- демо-лимит `5 запросов в день`
- защита лимита по устройству

## Как это должно работать в релизе

### Для тебя

Ты:
1. деплоишь backend на Railway
2. получаешь постоянный URL, например:

```text
https://your-app.up.railway.app
```

3. кладёшь этот адрес в файл:

[qt_client/backend_url.txt.example](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/qt_client/backend_url.txt.example)

В релизной сборке рядом с `Legalyze.exe` или `Legalyze` должен лежать файл:

```text
backend_url.txt
```

с одной строкой:

```text
https://your-app.up.railway.app
```

### Для пользователей

Пользователь:
1. скачивает приложение
2. запускает его
3. клиент сам берёт URL сервера из `backend_url.txt`

Пользователю не нужно:
- ставить Python
- запускать FastAPI
- создавать `.env`
- запускать backend локально

## Как теперь клиент ищет backend

Приоритет такой:

1. сохранённый адрес в настройках приложения
2. файл `backend_url.txt` рядом с приложением
3. переменная окружения `LEGALYZE_BACKEND_URL`
4. запасной локальный адрес `http://127.0.0.1:8000`

Для релиза тебе нужен именно пункт `2`.

## Что нужно подготовить перед релизом

### 1. Задеплоить backend на Railway

В репозитории уже есть:
- [railway.json](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/railway.json)
- [nixpacks.toml](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/nixpacks.toml)
- [backend/runtime.txt](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/backend/runtime.txt)
- [backend/.env.example](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/backend/.env.example)

### 2. Указать переменные окружения в Railway

Минимально:

```env
OPENAI_API_KEY=your_key
OPENAI_MODEL=gpt-4.1
APP_NAME=AI Legal Assistant API
API_VERSION=1.4.0
DATABASE_URL=sqlite:///./legal_assistant.db
UPLOAD_DIR=./storage/uploads
EMBEDDING_MODEL=text-embedding-3-small
EMBEDDING_DIMENSIONS=1536
RAG_TOP_K=5
RAG_CHUNK_SIZE=1200
RAG_CHUNK_OVERLAP=200
DEMO_DAILY_REQUEST_LIMIT=5
DONATION_URL=https://dalink.to/legalyze
DONATION_WIDGET_URL=https://www.donationalerts.com/widget/goal/9634016?token=TpCZoV1v6U7yj9LhPKWE
```

### 3. Проверить backend после деплоя

Открой:
- `https://your-app.up.railway.app/docs`
- `https://your-app.up.railway.app/landing`
- `https://your-app.up.railway.app/health`

## Как собрать клиент для пользователей

### Windows

Открой `qt_client/` в `Qt Creator`, выбери `Qt 6`, нажми `Configure Project`, потом `Run` или `Build`.

Либо используй:

```text
qt_client\build_windows.bat
```

После сборки:
1. возьми `Legalyze.exe`
2. положи рядом файл `backend_url.txt`
3. впиши туда свой Railway URL

### Linux

```bash
chmod +x qt_client/build_linux.sh
./qt_client/build_linux.sh
```

После сборки:
1. возьми бинарник `Legalyze`
2. положи рядом файл `backend_url.txt`
3. впиши туда свой Railway URL

## Что отдавать пользователям

### Windows

Папку или архив, внутри которого:
- `Legalyze.exe`
- `backend_url.txt`
- Qt-зависимости, если ты их положишь рядом через деплой Qt

### Linux

Папку или архив, внутри которого:
- `Legalyze`
- `backend_url.txt`
- нужные runtime-зависимости, если собираешь portable package

## Что НЕ нужно пользователям

Им не нужны:
- `backend/`
- Python
- `run_backend_windows.bat`
- `run_backend_linux.sh`
- `.env`
- Railway

Это всё только для тебя как владельца сервера.

## Важные замечания

- `SQLite` на Railway подходит только для демо
- для нормального публичного релиза лучше перейти на `PostgreSQL`
- для файлов потом лучше подключить внешнее хранилище
- временные tunnel URL лучше не использовать для релиза

## GitHub и релиз

Под релиз уже подготовлены:
- [LICENSE](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/LICENSE)
- [CHANGELOG.md](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/CHANGELOG.md)
- [RELEASE_CHECKLIST.md](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/RELEASE_CHECKLIST.md)
- [.gitignore](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/.gitignore)
- [.gitattributes](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/.gitattributes)
- [.github/FUNDING.yml](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/.github/FUNDING.yml)

Рекомендуемый первый тег:

```text
v0.1.0-demo
```

## Важное предупреждение

Legalyze — это информационный AI-ассистент.
Он не заменяет профессиональную юридическую консультацию, адвоката или официальное правовое заключение.
