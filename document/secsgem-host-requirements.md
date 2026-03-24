# SECS/GEM Host C++ 软件需求说明

- 文档版本：`v1.0`
- 编写日期：`2026-03-24`
- 适用范围：`SECS/GEM Host 端 C++ 软件，两阶段实施`
- 当前重点：`第一阶段，先在本地 WSL 与局域网设备间跑通 SEC/GEM 通信`

## 1. 文档目标

本文件用于定义 `SECS/GEM Host` 软件的需求边界、模块拆分、开发顺序与验收标准。

这份文档的主要用途有两个：

1. 作为后续“按模块生成代码”的开发依据。
2. 保证第一阶段先把 `HSMS / SECS-II / GEM` 的核心通信能力做扎实，再进入网页前端阶段。

## 2. 项目阶段规划

### 阶段一：先完成 Host 核心通信

目标是在 `WSL` 中运行 C++ Host 后端，并与局域网内设备端完成联调，至少做到：

- 建立 `HSMS-SS` 连接
- 完成 `SELECT` 与 `LINKTEST`
- 完成 `S1F13 / S1F14` 建立通信
- 读取 `ECID`
- 发送并接收 `Remote Command`
- 能记录通信日志，方便排查问题
- 提供最小调试入口，便于后续前端接入

阶段一交付物：

- 一个可以在 `WSL / Linux` 运行的 `C++ Host Service`
- 一套清晰的配置文件
- 一套最小调试接口，建议包含 `CLI` 或简单 `REST API`
- 一套联调文档和验收用例

### 阶段二：增加网页前端

目标是在第一阶段稳定的基础上，加一个网页前端，通过 `HTTP / WebSocket` 与 C++ 后端通信，提供：

- 设备连接状态查看
- ECID 查询界面
- Command 发送界面
- 报文收发实时显示
- 错误与超时提示

阶段二交付物：

- 浏览器可访问的前端页面
- 前后端接口文档
- 基础操作页面与日志页面

## 3. 总体技术假设

以下假设是本项目落地的前提，后续代码实现默认基于这些条件：

1. 通信协议以 `HSMS-SS over TCP/IP` 为主，不考虑 `SECS-I` 串口版本。
2. Host 后端首先运行在 `WSL` 或 `Ubuntu Linux` 中。
3. 设备端位于同一局域网，可通过 `IP + Port` 访问。
4. 第一阶段优先支持 `Host Active` 模式，即 Host 主动连接设备。
5. 如果设备要求 `Passive` 模式，则需要额外处理 `WSL` 端口暴露、Windows 防火墙和端口转发，这属于第一阶段可选扩展。
6. 第一阶段先以“单设备稳定联调”为第一目标，但架构应保留多设备扩展能力。
7. 第二阶段前端不得直接实现 `HSMS / SECS-II / GEM`，所有协议逻辑必须集中在 C++ 后端。

## 4. 第一阶段详细需求

### 4.1 阶段目标

第一阶段不是做 UI，而是做一个“真正能和设备讲话”的 Host 内核。

第一阶段完成后，应该达到下面的实际效果：

- 在 `WSL` 中启动 Host 程序
- 从 Host 连接设备端的 `HSMS` 地址
- 看到 `TCP 连接 -> HSMS Select -> GEM 通信建立` 的完整过程
- 通过命令读取指定 `ECID`
- 通过命令下发 `RCMD`
- 在日志中看到请求报文、响应报文、超时和错误原因
- 当网络断开或设备重启时，Host 能给出清晰状态变化

### 4.2 第一阶段网络拓扑

推荐先按下面的方式联调：

```text
+----------------------+        TCP / HSMS-SS        +----------------------+
| Windows 主机         | <-------------------------> | 局域网设备端         |
|   └─ WSL2 Ubuntu     |                             | Equipment / Simulator |
|       └─ C++ Host    |                             | IP: x.x.x.x:port      |
+----------------------+                             +----------------------+
```

推荐的第一版联调模式：

- `Host Active`
- `WSL -> 设备端 IP:Port` 主动发起 TCP 连接
- 暂不要求设备先连入 Host

原因：

- `WSL -> LAN` 的主动出站连接最简单
- 不需要先处理 `Windows -> WSL` 入站映射
- 更适合先把协议栈调通

### 4.3 第一阶段范围

#### 必做范围

1. 设备配置加载
2. TCP 建链与断链
3. `HSMS Select / Deselect / Linktest / Separate`
4. `SECS-II` 报文编解码
5. `S1F13 / S1F14` 建立通信
6. `S2F13 / S2F14` 读取 `ECID`
7. `S2F41 / S2F42` 发送设备命令
8. 请求与响应的 `system bytes` 关联
9. 超时控制与错误处理
10. 报文日志与状态日志
11. 一个最小调试接口，便于手工触发 `connect / ecid / command`

#### 建议做范围

1. 自动重连
2. 可配置 `T3 / T5 / T6 / T7 / T8`
3. 设备多实例配置
4. 原始十六进制报文落盘
5. 简单 `REST API`，为第二阶段前端提前铺路

#### 第一阶段暂不做

1. 完整的 GEM 报表定义配置 `RPTID / CEID / VID`
2. 报警管理全流程
3. 配方管理
4. 用户权限系统
5. 数据库存储
6. 复杂前端页面
7. 云端部署优化

### 4.4 协议能力要求

#### 4.4.1 HSMS 层

Host 必须支持以下控制报文：

- `SELECT_REQ`
- `SELECT_RSP`
- `DESELECT_REQ`
- `DESELECT_RSP`
- `LINKTEST_REQ`
- `LINKTEST_RSP`
- `SEPARATE_REQ`

Host 必须具备以下状态管理：

- `DISCONNECTED`
- `TCP_CONNECTED`
- `SELECTED`
- `COMMUNICATING`
- `RECONNECT_WAIT`
- `ERROR`

Host 必须实现以下超时与重试逻辑：

- `T3`：事务响应超时
- `T5`：连接重试等待
- `T6`：控制事务超时
- `T7`：连接建立后等待 Select 完成
- `T8`：网络收包间隔超时

推荐默认值：

- `T3 = 45s`
- `T5 = 10s`
- `T6 = 5s`
- `T7 = 10s`
- `T8 = 5s`
- `linktest_interval = 30s`

说明：

- 实际设备可能要求不同超时，必须做成配置项。
- `system bytes` 必须由 Host 自增生成，并用于匹配主请求与响应。
- 收到非预期控制报文时，Host 不能崩溃，必须记录日志并进入明确状态。

#### 4.4.2 SECS-II 编解码层

第一阶段至少支持以下常见数据类型：

- `L`
- `A`
- `B`
- `BOOLEAN`
- `I1 / I2 / I4 / I8`
- `U1 / U2 / U4 / U8`
- `F4 / F8`

编解码要求：

1. 能从字节流解出 `stream / function / wbit / system bytes / body`
2. 能把结构化对象编码回标准 `SECS-II` 二进制
3. 能输出便于日志记录的可读文本
4. 发生非法格式时，返回明确错误，而不是静默吞掉
5. 保留原始字节数组，便于抓包对比

#### 4.4.3 GEM 业务层

第一阶段 GEM 层最少支持以下业务动作：

1. 建立通信
2. 读取 `ECID`
3. 发送 `Remote Command`
4. 接收并记录设备主动上报的消息

建议优先支持的业务报文如下：

| 场景 | Host 发送 | 设备返回 | 是否必做 |
| --- | --- | --- | --- |
| 通信建立 | `S1F13` | `S1F14` | 是 |
| 在线确认，按设备要求启用 | `S1F17` | `S1F18` | 可选 |
| 存活确认 | `LINKTEST_REQ` | `LINKTEST_RSP` | 是 |
| 读取 ECID | `S2F13` | `S2F14` | 是 |
| 下发命令 | `S2F41` | `S2F42` | 是 |
| 设备事件上报 | `S6F11` | 视设备配置 | 建议接收 |
| 报警上报 | `S5F1` | `S5F2` | 建议接收 |

说明：

- `ECID` 的标准读取通常使用 `S2F13 / S2F14`。
- 设备命令一般使用 `S2F41 / S2F42`。
- 实际 `RCMD` 名称、参数名、参数类型必须以设备厂家手册为准。

### 4.5 第一阶段功能需求细化

#### 4.5.1 配置管理

Host 程序必须支持从配置文件读取以下内容：

- 设备唯一标识 `device_id`
- 设备显示名 `device_name`
- `ip`
- `port`
- `connect_mode`，支持 `active`，可预留 `passive`
- `session_id`
- 各类超时参数
- `linktest_interval`
- 自动重连开关
- 默认 `ECID` 列表
- 支持的 `RCMD` 定义
- 日志目录

建议配置示例：

```json
{
  "devices": [
    {
      "device_id": "eqp01",
      "device_name": "EQP-01",
      "ip": "192.168.1.50",
      "port": 5000,
      "connect_mode": "active",
      "session_id": 1,
      "timeouts": {
        "t3_sec": 45,
        "t5_sec": 10,
        "t6_sec": 5,
        "t7_sec": 10,
        "t8_sec": 5
      },
      "linktest_interval_sec": 30,
      "auto_reconnect": true,
      "default_ecids": [1001, 1002, 2001],
      "commands": [
        {
          "name": "START",
          "params": ["LOTID", "RECIPE"]
        },
        {
          "name": "STOP",
          "params": []
        }
      ]
    }
  ]
}
```

#### 4.5.2 会话管理

每台设备需要一个独立会话对象，负责：

- 连接建立
- Select 过程
- 事务发送
- 响应匹配
- 心跳维护
- 异常断开检测
- 自动重连

一个会话在程序内部应当能查询以下信息：

- 当前连接状态
- 最近一次连接时间
- 最近一次断开原因
- 最近一次发送报文
- 最近一次接收报文
- 当前未完成事务数量

#### 4.5.3 ECID 读取

Host 必须支持两种方式读取 `ECID`：

1. 指定 `ECID` 列表读取
2. 按配置中的默认 `ECID` 批量读取

执行结果必须返回：

- 请求时间
- 响应时间
- 请求使用的 `system bytes`
- 每个 `ECID` 的返回值
- 每个 `ECID` 的数据类型
- 失败原因或超时原因

#### 4.5.4 Remote Command

Host 必须支持发送 `S2F41` 命令，至少满足：

- 指定 `RCMD`
- 携带命令参数
- 支持字符串和整数参数
- 返回 `HCACK`
- 如设备返回参数级错误，需要能展示

返回结果至少包含：

- `RCMD`
- 参数列表
- 发送时间
- 响应时间
- `HCACK`
- 设备附加错误信息

#### 4.5.5 调试入口

第一阶段必须至少提供一种“人可以直接操作”的调试入口。

推荐实现方式：

1. `CLI` 命令行
2. 简单 `REST API`

推荐最小调试动作：

- `connect <device_id>`
- `disconnect <device_id>`
- `status <device_id>`
- `linktest <device_id>`
- `read-ecid <device_id> <ecid...>`
- `read-default-ecid <device_id>`
- `send-command <device_id> <rcmd> [key=value ...]`
- `tail-messages <device_id>`

如果采用 `REST API`，建议最少提供：

- `GET /api/devices`
- `POST /api/devices/{id}/connect`
- `POST /api/devices/{id}/disconnect`
- `POST /api/devices/{id}/linktest`
- `POST /api/devices/{id}/ecids/read`
- `POST /api/devices/{id}/commands/execute`
- `GET /api/devices/{id}/messages`
- `GET /ws/devices/{id}`

这套接口与当前仓库已有的前后端骨架方向一致，后续第二阶段可以直接复用。

### 4.6 非功能需求

#### 4.6.1 运行环境

- 语言：`C++17`
- 构建：`CMake`
- 第一阶段目标运行环境：`WSL Ubuntu` 或标准 `Linux`
- 编译结果：单独可执行后端程序

#### 4.6.2 稳定性

要求：

1. 设备断线、超时、非法包不能导致进程崩溃
2. 请求失败后必须返回明确错误
3. 日志必须能定位到是哪一条事务失败
4. 多次连接和断开后不能出现明显资源泄漏

#### 4.6.3 可观测性

日志至少分三类：

1. 会话状态日志
2. 协议报文日志
3. 业务操作日志

每条关键日志建议包含：

- 时间戳
- 设备 ID
- 方向 `send / recv / local`
- `stream/function`
- `system bytes`
- 摘要说明
- 原始十六进制，按配置开关启用

#### 4.6.4 可扩展性

代码结构必须支持后续增加：

- 多设备并发
- 更多 `SxFy`
- 前端 Web 接口
- 日志持久化
- 设备差异化适配

### 4.7 推荐模块拆分

下面的模块拆分建议作为后续逐模块生成代码的顺序基础。

| 模块 | 责任 | 建议文件 |
| --- | --- | --- |
| 配置模块 | 读取设备配置、超时、命令定义 | `backend/include/app_config.h`, `backend/src/app_config.cpp` |
| 日志模块 | 统一日志输出、十六进制转储 | `backend/include/logger.h`, `backend/src/logger.cpp` |
| 基础类型模块 | 会话状态、错误码、事务对象 | `backend/include/core_types.h`, `backend/src/core_types.cpp` |
| 字节缓冲模块 | 大小端转换、编码辅助 | `backend/include/byte_buffer.h`, `backend/src/byte_buffer.cpp` |
| HSMS 帧模块 | HSMS 头部、控制类型、打包拆包 | `backend/include/hsms_types.h`, `backend/src/hsms_types.cpp` |
| TCP 传输模块 | socket 连接、读写、重连 | `backend/include/tcp_client.h`, `backend/src/tcp_client.cpp` |
| HSMS 会话模块 | select、linktest、状态机、超时 | `backend/include/hsms_session.h`, `backend/src/hsms_session.cpp` |
| SECS-II 编解码模块 | item 编码、消息解码、可读化 | `backend/include/secs_codec.h`, `backend/src/secs_codec.cpp` |
| 事务管理模块 | `system bytes` 分配、请求响应匹配 | `backend/include/transaction_manager.h`, `backend/src/transaction_manager.cpp` |
| GEM Host 服务模块 | `S1F13/S1F14`, `S2F13/S2F14`, `S2F41/S2F42` | `backend/include/gem_host_service.h`, `backend/src/gem_host_service.cpp` |
| 设备管理模块 | 多设备实例、统一生命周期管理 | `backend/include/device_manager.h`, `backend/src/device_manager.cpp` |
| 调试接口模块 | CLI 或 REST 调试入口 | `backend/include/debug_api.h`, `backend/src/debug_api.cpp` |
| 程序入口 | 组装配置、启动服务 | `backend/src/main.cpp` |

补充建议：

- `tcp_client` 和 `hsms_session` 要分开，避免网络层和协议层耦合。
- `secs_codec` 要尽量纯粹，不直接依赖 socket。
- `gem_host_service` 只组织 GEM 业务动作，不负责底层收发。
- 第二阶段网页接口建议建立在 `device_manager + gem_host_service` 之上。

### 4.8 推荐开发顺序

建议按下面顺序在下一个对话中逐模块生成代码：

1. 配置模块
2. 日志模块
3. 基础类型与字节缓冲模块
4. HSMS 帧定义模块
5. TCP 传输模块
6. HSMS 会话状态机模块
7. SECS-II 编解码模块
8. 事务管理模块
9. GEM Host 服务模块
10. 设备管理模块
11. 最小调试接口
12. 集成测试与联调脚本

这样拆的原因：

- 先把最底层数据结构打稳
- 再做网络与状态机
- 再做协议编解码
- 最后做业务命令和调试入口

### 4.9 第一阶段验收标准

只要下面项目全部通过，就认为第一阶段完成。

#### 4.9.1 连接验收

1. Host 能从 `WSL` 连接设备 `IP:Port`
2. 能成功完成 `SELECT_REQ / SELECT_RSP`
3. 连接成功后状态进入 `SELECTED` 或 `COMMUNICATING`
4. 主动断开和被动断开都能正确记录

#### 4.9.2 心跳验收

1. 能发送 `LINKTEST_REQ`
2. 能收到 `LINKTEST_RSP`
3. 超时后能记录错误并触发重连或状态更新

#### 4.9.3 建立通信验收

1. Host 能发送 `S1F13`
2. 能收到设备返回的 `S1F14`
3. 日志中能看到完整事务与 `system bytes`

#### 4.9.4 ECID 验收

1. 能读取单个 `ECID`
2. 能批量读取多个 `ECID`
3. 能正确解析 `S2F14` 返回值
4. 对不存在的 `ECID` 或非法响应能返回明确错误

#### 4.9.5 Command 验收

1. 能发送 `S2F41`
2. 能收到 `S2F42`
3. 能解析 `HCACK`
4. 参数错误时能看到设备返回的失败信息

#### 4.9.6 日志验收

1. 每次请求和响应都能记录时间戳
2. 每条事务都能关联 `system bytes`
3. 关键错误都有原因说明
4. 可选输出原始十六进制报文

### 4.10 联调测试用例建议

建议至少准备以下测试用例：

| 编号 | 用例 | 预期结果 |
| --- | --- | --- |
| TC-01 | Host 启动后连接设备 | TCP 建立成功，完成 Select |
| TC-02 | Host 手工执行 Linktest | 收到 Linktest 响应 |
| TC-03 | Host 执行 `S1F13` | 收到 `S1F14` |
| TC-04 | Host 读取单个 `ECID` | 返回正确值 |
| TC-05 | Host 批量读取多个 `ECID` | 返回完整列表 |
| TC-06 | Host 发送合法 `RCMD` | `HCACK=0` 或设备定义的成功码 |
| TC-07 | Host 发送非法参数命令 | 返回明确失败码，不崩溃 |
| TC-08 | 联调过程中拔网线或杀掉设备服务 | Host 状态变更明确，可重连 |
| TC-09 | 设备主动上报消息 | Host 能收并记日志 |

### 4.11 设备资料前置要求

真正开始和设备联调前，必须尽量拿到以下资料，否则后续代码只能先做通用框架：

1. 设备 `HSMS` 通信模式：`active` 还是 `passive`
2. 设备 `IP` 与 `Port`
3. 设备要求的 `session_id`
4. 设备支持的 `ECID` 列表、名称、数据类型
5. 设备支持的 `RCMD` 列表、参数名、参数类型
6. 是否要求 `S1F17 / S1F18` 在线切换
7. 是否会主动发送 `S5F1 / S6F11` 等消息
8. 厂家给出的报文示例或抓包样例

这部分资料越完整，后续代码生成越容易一次写对。

### 4.12 第一阶段风险点

1. 设备厂商对标准有私有扩展，不能只按通用标准猜实现。
2. `ECID` 和 `RCMD` 的参数类型如果未知，编解码很容易写错。
3. 如果设备要求 `Passive` 模式，`WSL` 的网络暴露会明显复杂一些。
4. 如果第一阶段过早做前端，会拖慢核心协议联调。

因此第一阶段必须坚持一个原则：

`先把 Host 内核和设备联调跑通，再做网页界面。`

## 5. 第二阶段需求概览

第二阶段建立在第一阶段稳定后端之上，网页只做“控制台”和“可视化”，不重复实现协议。

### 5.1 第二阶段目标

- 提供设备列表页面
- 提供连接控制页面
- 提供 `ECID` 查询页面
- 提供 `Command` 下发页面
- 提供报文实时查看页面
- 提供错误提示和连接状态变化提示

### 5.2 第二阶段后端接口建议

后端继续由 C++ 提供：

- `REST API`：触发查询和命令
- `WebSocket`：推送设备状态和报文事件

前端只做：

- 页面展示
- 表单录入
- 调用 `REST API`
- 订阅 `WebSocket`

### 5.3 第二阶段页面建议

1. 设备总览页
2. 单设备会话页
3. ECID 查询页
4. Remote Command 页
5. 报文日志页

## 6. 下一轮代码生成建议

在下一个对话中，建议直接按下面顺序让我逐模块生成代码：

1. 先生成 `配置模块`
2. 再生成 `日志模块`
3. 再生成 `HSMS 基础类型 + 帧定义`
4. 再生成 `TCP 传输模块`
5. 再生成 `HSMS 会话状态机`
6. 再生成 `SECS-II 编解码`
7. 再生成 `ECID 读取能力`
8. 再生成 `Remote Command 能力`
9. 最后生成 `调试接口`

如果要提高一次成功率，建议你在下一个对话开始时同时提供以下信息：

- 设备的 `IP / Port`
- 设备是 `Active` 还是 `Passive`
- 几个真实 `ECID`
- 至少一个真实 `RCMD` 和参数定义
- 是否已经有厂家报文样例

---

这份文档的核心结论只有一句话：

`第一阶段的成功标准，不是页面好看，而是 Host 在 WSL 中能稳定连上局域网设备、读出 ECID、下发命令，并把整个事务过程记录清楚。`
