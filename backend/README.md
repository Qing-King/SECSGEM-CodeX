# Backend Skeleton

后端建议实现为独立的 `C++ Host Service`，对外提供：

- `REST API`：设备配置、连接控制、日志查询
- `WebSocket`：通信状态、报文、报警、事件实时推送

建议模块拆分：

- `src/main.cpp`：程序入口
- `src/device_manager.*`：设备配置和会话管理
- `src/host_session.*`：HSMS 会话控制
- `src/secs_codec.*`：SECS-II 编解码
- `src/gem_service.*`：GEM 业务逻辑
- `src/message_store.*`：消息缓存与持久化

后续可以选择：

- `Drogon` 作为 HTTP / WebSocket 服务框架
- 或 `Boost.Asio / Beast` 自行实现网络层
