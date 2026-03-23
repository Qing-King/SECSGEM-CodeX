# SECSGEM-CodeX

这是一个面向 `SEC/GEM Host` 的最小起步仓库，目标是构建：

- `Vue 3 + Vite` 网页控制台
- `C++` Host 后端服务
- `REST + WebSocket` 前后端通信
- 后续逐步补 `HSMS / SECS-II / GEM` 协议实现

当前第一阶段已包含：

- 一个设计文档页面 `document/design-overview.html`
- 前端项目骨架目录
- 后端项目骨架目录

建议推进顺序：

1. 先把前端壳子跑起来
2. 再补后端 API 和 WebSocket
3. 最后接入真实 SEC/GEM 协议栈
