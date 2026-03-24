# SECSGEM-CodeX

这是一个面向 `SEC/GEM Host` 的最小起步仓库，当前包含：

- `Vue 3 + Vite` 网页控制台
- `C++` Host 后端服务
- `REST + WebSocket` 前后端通信骨架
- 云服务器部署文档和 WSL 本地开发文档

文档入口：

- `document/design-overview.html`
- `document/cloud-deploy.html`
- `document/wsl-dev.html`
- `document/todo-board.html`

## 环境要求

- 前端：`Node.js >= 20.19.0`
- 后端：`CMake >= 3.16`
- 编译器：支持 `C++17`

## WSL 本地开发

前端启动：

```bash
cd /mnt/c/Build/SECSGEM-CODEX/SECSGEM-CodeX/frontend
nvm use 20
npm install
npm run dev -- --host 0.0.0.0
```

后端启动：

```bash
cd /mnt/c/Build/SECSGEM-CODEX/SECSGEM-CodeX/backend
cmake -S . -B build
cmake --build build -j
./build/secsgem_backend
```

浏览器访问：

```text
http://localhost:5173
http://localhost:8080/api/devices
http://localhost:8080/api/devices/eqp01/messages
```

## Ubuntu 云服务器部署

一键部署：

```bash
cd /opt/secsgem-codex
git pull
chmod +x deploy-ubuntu.sh
./deploy-ubuntu.sh
```

后端 service 安装：

```bash
sudo cp secsgem-backend.service.example /etc/systemd/system/secsgem-backend.service
sudo systemctl daemon-reload
sudo systemctl enable secsgem-backend
sudo systemctl restart secsgem-backend
```

Nginx 安装与重启：

```bash
sudo apt update
sudo apt install -y nginx
sudo systemctl enable nginx
sudo systemctl restart nginx
```

浏览器访问：

```text
http://YOUR_PUBLIC_IP/
http://YOUR_PUBLIC_IP/docs/design-overview.html
http://YOUR_PUBLIC_IP/docs/cloud-deploy.html
http://YOUR_PUBLIC_IP/docs/wsl-dev.html
http://YOUR_PUBLIC_IP/docs/todo-board.html
http://YOUR_PUBLIC_IP/api/devices
http://YOUR_PUBLIC_IP/api/devices/eqp01/messages
```

## 当前已接通的最小接口

- `GET /api/devices`
- `GET /api/devices/{id}/messages`
- `POST /api/devices/{id}/connect`
- `POST /api/devices/{id}/disconnect`
- `POST /api/devices/{id}/linktest`
- `GET /ws/devices/{id}`

## 推荐推进顺序

1. 先用 WSL 跑通前后端联调
2. 再同步到 Ubuntu 云服务器
3. 然后把 WebSocket 推送对接到真实 `HSMS / SECS-II / GEM` 事件源
4. 最后补完整会话管理、日志和持久化
