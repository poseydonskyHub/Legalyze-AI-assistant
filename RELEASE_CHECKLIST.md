# Release Checklist

## Before GitHub Release

- [ ] Backend runs locally with `.env`
- [ ] Qt client builds on the target OS
- [ ] `POST /auth/register` works
- [ ] `POST /auth/login` works
- [ ] `POST /v1/chat` works
- [ ] `POST /v1/documents/upload` works
- [ ] `POST /v1/rag/query` works
- [ ] Demo limit is enforced
- [ ] Landing page opens
- [ ] Donation link opens

## Before Railway Deploy

- [ ] GitHub repo is clean
- [ ] `.env` is not committed
- [ ] `backend/.env.example` is up to date
- [ ] `railway.json` is committed
- [ ] `nixpacks.toml` is committed
- [ ] `OPENAI_API_KEY` is ready
- [ ] Railway variables are filled in
- [ ] SQLite is understood as demo-only on Railway

## After Railway Deploy

- [ ] `/docs` opens
- [ ] `/health` returns `ok`
- [ ] `/landing` opens
- [ ] Login from desktop client works
- [ ] Chat returns answers

## Before Public Sharing

- [ ] README is updated
- [ ] Screenshots are ready
- [ ] Version tag created
- [ ] Demo disclaimer is visible
- [ ] GitHub funding link points to the correct donation page
