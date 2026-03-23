# Deployment Guide

## 推荐方式

Windows 开发机负责写代码，Ubuntu 云服务器负责运行和对外访问。

推荐部署链路：

1. Windows 本地开发 `frontend` 和 `backend`
2. 代码推到 Git 仓库
3. Ubuntu 云服务器拉取最新代码
4. 云服务器运行前端构建产物和后端服务
5. Nginx 暴露 `80/443`，浏览器通过公网 IP 访问

## 为什么不建议“实时同步文件夹”

开发初期最快的是 `git push -> git pull`。
真正的“实时同步文件改了就自动上线”可以做，但不建议一开始就这样做，因为：

- 容易把半成品直接同步到线上
- 出错后不好回滚
- 前端依赖和后端编译常常需要单独处理

## 你现在最适合的流程

### 方案 A：Git 手动发布

Windows:

```powershell
git add .
git commit -m "update ui and api contract"
git push origin main
```

Ubuntu:

```bash
cd /opt/secsgem-codex
git pull
cd frontend
npm install
npm run build
```

### 方案 B：Git + 自动拉取脚本

```bash
#!/usr/bin/env bash
set -e
cd /opt/secsgem-codex
git pull
cd frontend
npm install
npm run build
sudo systemctl restart secsgem-backend
sudo systemctl reload nginx
```

## 访问方式

假设公网 IP 是 `1.2.3.4`：

- 前端页面：`http://1.2.3.4/`
- 设计文档：`http://1.2.3.4/docs/design-overview.html`

## Nginx 示例

```nginx
server {
    listen 80;
    server_name _;

    root /var/www/secsgem-console/dist;
    index index.html;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /docs/ {
        alias /var/www/secsgem-console/docs/;
        index design-overview.html;
    }

    location /api/ {
        proxy_pass http://127.0.0.1:8080/;
    }

    location /ws/ {
        proxy_pass http://127.0.0.1:8080/ws/;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```
