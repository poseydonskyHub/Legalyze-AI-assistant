# Legalyze Qt Client

Этот клиент рассчитан на схему:
- один backend у владельца проекта
- все пользователи подключаются к нему через готовый desktop-клиент

## Главное

Обычному пользователю **не нужно** запускать backend локально.

Чтобы клиент работал у всех пользователей, рядом с приложением должен лежать файл:

```text
backend_url.txt
```

Пример содержимого:

```text
https://your-app.up.railway.app
```

Шаблон лежит здесь:
- [backend_url.txt.example](/C:/Users/Марк/Documents/Codex/2026-05-17/fastapi/qt_client/backend_url.txt.example)

## Как клиент выбирает сервер

Приоритет такой:

1. сохранённый адрес
2. `backend_url.txt` рядом с приложением
3. `LEGALYZE_BACKEND_URL`
4. `http://127.0.0.1:8000`

Для релиза тебе нужен именно `backend_url.txt`.

## Сборка на Windows

Простой вариант:

```text
build_windows.bat
```

Или через `Qt Creator`:
- открыть `qt_client`
- выбрать `Qt 6`
- нажать `Configure Project`
- собрать проект

## Сборка на Linux

Простой вариант:

```bash
chmod +x build_linux.sh
./build_linux.sh
```

## Что выкладывать пользователю

### Windows

- `Legalyze.exe`
- `backend_url.txt`
- Qt runtime файлы

### Linux

- `Legalyze`
- `backend_url.txt`
- нужные runtime зависимости или portable package

## Что пользователю не нужно

- backend
- Python
- `.env`
- Railway
- OpenAI API key
