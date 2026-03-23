# Ubuntu Runbook

Set one variable first:

```bash
export APP_DIR=/opt/secsgem-codex
```

## 1. Install packages

```bash
sudo apt update
sudo apt install -y git nginx nodejs npm cmake g++
```

If `nginx.service` is missing later, verify installation with:

```bash
systemctl list-unit-files | grep nginx
which nginx
```

## 2. Clone the project

```bash
sudo mkdir -p /opt
cd /opt
sudo git clone <YOUR_GIT_REPO_URL> secsgem-codex
sudo chown -R $USER:$USER $APP_DIR
cd $APP_DIR
```

## 3. Build frontend

```bash
cd $APP_DIR/frontend
npm install
npm run build
```

## 4. Build backend

```bash
cd $APP_DIR/backend
cmake -S . -B build
cmake --build build -j
```

## 5. Prepare web root

```bash
sudo mkdir -p /var/www/secsgem-console/docs
sudo rm -rf /var/www/secsgem-console/dist
sudo cp -r $APP_DIR/frontend/dist /var/www/secsgem-console/dist
sudo cp $APP_DIR/document/design-overview.html /var/www/secsgem-console/docs/design-overview.html
sudo cp $APP_DIR/document/todo-board.html /var/www/secsgem-console/docs/todo-board.html
```

## 6. Install nginx config

```bash
sudo cp $APP_DIR/nginx-secsgem.conf.example /etc/nginx/sites-available/secsgem
sudo ln -sf /etc/nginx/sites-available/secsgem /etc/nginx/sites-enabled/secsgem
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t
sudo systemctl restart nginx
```

Important:

- `proxy_pass` must be `http://127.0.0.1:8080`
- do not add a trailing `/`
- otherwise `/api/devices` may be rewritten to `/devices`

## 7. Run backend manually for first test

```bash
cd $APP_DIR/backend/build
./secsgem_backend
```

## 7b. Optional systemd service

```bash
sudo cp $APP_DIR/secsgem-backend.service.example /etc/systemd/system/secsgem-backend.service
sudo systemctl daemon-reload
sudo systemctl enable secsgem-backend
sudo systemctl start secsgem-backend
```

## 8. Verify from browser

```text
http://YOUR_PUBLIC_IP/
http://YOUR_PUBLIC_IP/docs/design-overview.html
http://YOUR_PUBLIC_IP/docs/todo-board.html
http://YOUR_PUBLIC_IP/api/devices
http://YOUR_PUBLIC_IP/api/devices/eqp01/messages
```

## 9. Update after local changes

Windows:

```powershell
git add .
git commit -m "update ui or backend"
git push
```

Ubuntu:

```bash
cd $APP_DIR
./deploy-ubuntu.sh
```
