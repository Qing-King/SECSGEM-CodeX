# Session Handoff

Date: 2026-03-24

## Project Goal

Build a `SEC/GEM Host` web console with:

- Frontend: `Vue 3 + Vite + Element Plus + ECharts`
- Backend: `C++`
- Transport: `REST + WebSocket`
- Deploy target: Ubuntu cloud server
- Local dev target: WSL

## Repository

- Remote: `git@github.com:Qing-King/SECSGEM-CodeX.git`
- Active branch: `codex/secsgem-web-host-skeleton`

## Recent Commits

```text
9723af9 Document startup commands and Node requirements
f461a3b Make deploy script path-dynamic
e130fbb Split cloud and WSL deployment docs
26f3b2e Wire frontend to live API endpoints
b760dfe Add quick links to design overview
b9fe437 Fix nginx deployment and proxy config
84a17b1 Add SEC/GEM web console and deployment skeleton
```

## What Was Built

### Frontend

- Runtime page in `frontend/src/App.vue`
- Components:
  - `DevicePanel.vue`
  - `ConnectionToolbar.vue`
  - `AlarmPanel.vue`
  - `MessageTable.vue`
  - `MessageDetailDrawer.vue`
  - `ArchitectureCard.vue`
- Real API integration with fallback to mock:
  - `GET /api/devices`
  - `GET /api/devices/{id}/messages`
  - `POST /api/devices/{id}/connect`
  - `POST /api/devices/{id}/disconnect`
  - `POST /api/devices/{id}/linktest`
- Real WebSocket client:
  - `GET /ws/devices/{id}` upgrade
  - Reconnect on active-device switch
  - Apply `session_state` and `secs_message` events to the runtime page

### Backend

- Minimal Linux-oriented HTTP server in:
  - `backend/src/http_server.cpp`
  - `backend/src/main.cpp`
- Current routes:
  - `GET /api/devices`
  - `GET /api/devices/{id}/messages`
  - `POST /api/devices/{id}/connect`
  - `POST /api/devices/{id}/disconnect`
  - `POST /api/devices/{id}/linktest`
  - `GET /ws/devices/{id}`
- In-memory device state and message history:
  - connect/disconnect updates device status
  - linktest appends a live message event
  - action results are broadcast over WebSocket

### Docs

- `document/design-overview.html`
- `document/todo-board.html`
- `document/cloud-deploy.html`
- `document/wsl-dev.html`

## Current State

### Working

- WSL local frontend can run after upgrading to `Node 20`
- WSL backend can build and run
- Ubuntu deployment script exists
- Nginx config exists
- Design docs are separated from runtime UI
- Frontend now connects to a real backend WebSocket endpoint
- Vite dev server proxies `/api` and `/ws` to `127.0.0.1:8080`

### Not Finished Yet

- No real `HSMS / SECS-II / GEM` protocol implementation yet
- Backend is still a minimal in-memory socket server, not a full framework server
- WebSocket events are still synthesized from REST actions, not from a real equipment session

## Start Commands

### WSL Local Dev

Frontend:

```bash
cd /mnt/c/Build/SECSGEM-CODEX/SECSGEM-CodeX/frontend
nvm use 20
npm install
npm run dev -- --host 0.0.0.0
```

Backend:

```bash
cd /mnt/c/Build/SECSGEM-CODEX/SECSGEM-CodeX/backend
cmake -S . -B build
cmake --build build -j
./build/secsgem_backend
```

### Ubuntu Cloud

```bash
cd /opt/secsgem-codex
git pull
chmod +x deploy-ubuntu.sh
./deploy-ubuntu.sh
sudo systemctl restart secsgem-backend
sudo systemctl restart nginx
```

## URLs

### Runtime

- `/`
- `/api/devices`
- `/api/devices/eqp01/messages`
- `/ws/devices/eqp01`

### Docs

- `/docs/design-overview.html`
- `/docs/todo-board.html`
- `/docs/cloud-deploy.html`
- `/docs/wsl-dev.html`

## Recommended Next Steps

1. Replace synthetic WebSocket events with real `HSMS / SECS-II / GEM` session events.
2. Add structured backend logging for REST and WebSocket activity.
3. Add persistent message/alarm storage instead of in-memory vectors.
4. Introduce a more complete server foundation if the backend needs routing/middleware growth.
5. Add end-to-end verification in WSL/Ubuntu once Node and Linux build toolchains are available.

## Resume Checklist On A New Computer

1. Clone the repo.
2. Checkout `codex/secsgem-web-host-skeleton`.
3. Read this file and `README.md`.
4. If using WSL, install `nvm` and use Node 20.
5. Start backend and frontend with the commands above.
6. Continue from real protocol integration and persistence.

