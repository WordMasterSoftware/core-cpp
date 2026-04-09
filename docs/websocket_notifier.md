# WebSocketNotifier 使用说明

## 作用

`wmnext::ws::WebSocketNotifier` 用于统一管理 WebSocket 连接，并向所有已连接客户端主动推送消息。

假设服务监听地址为：

- WebSocket 端口：`8182`
- 订阅路径：`/queue`
- 完整地址示例：`ws://127.0.0.1:8182/queue`

这个封装适合在服务端业务代码中直接调用，避免每个模块自己处理连接、广播和消息序列化。

## 头文件

```cpp
#include "wmnext/ws/websocket_notifier.hpp"
```

## 生命周期

通常由应用启动层创建并启动，然后把 `WebSocketNotifier` 以引用形式传给需要推送消息的业务模块。

当前项目中的装配位置在：

- `core-cpp/src/app/server_application.cpp`

启动方式：

```cpp
ws::WebSocketNotifier websocket_notifier;

if (!websocket_notifier.start("0.0.0.0", 8182)) {
    // 启动失败处理
}
```

停止方式：

```cpp
websocket_notifier.stop();
```

## 对外接口

### 1. 按事件名和负载推送

```cpp
websocket_notifier.notify(
    "order.created",
    http::Json{
        {"order_id", 12345},
        {"status", "created"}
    }
);
```

发送给客户端的消息结构为：

```json
{
  "event": "order.created",
  "payload": {
    "order_id": 12345,
    "status": "created"
  }
}
```

这是推荐用法。业务语义更清晰，也方便前端按 `event` 分发处理。

### 2. 直接推送完整消息体

```cpp
websocket_notifier.notify(http::Json{
    {"event", "custom.message"},
    {"payload", {
        {"value", 1}
    }},
    {"meta", {
        {"source", "server"}
    }}
});
```

这种方式适合你需要完全自定义消息结构时使用。

## 在业务模块中接入

推荐做法是把 `WebSocketNotifier&` 作为参数注入到注册函数、服务类或业务处理函数中。

例如当前 `POST /add` 的实现就是这样接入的：

```cpp
void register_math_api(httplib::Server& server, ws::WebSocketNotifier& websocket_notifier) {
    server.Post("/add", [&websocket_notifier](const httplib::Request& request, httplib::Response& response) {
        const double result = 1 + 2;

        websocket_notifier.notify(
            "math.add.completed",
            http::Json{
                {"result", result}
            }
        );
    });
}
```

## 客户端接收示例

浏览器端示例：

```javascript
const socket = new WebSocket("ws://127.0.0.1:8182/queue");

socket.onmessage = (event) => {
  const message = JSON.parse(event.data);
  console.log("event:", message.event);
  console.log("payload:", message.payload);
};
```

## 使用建议

- 推荐统一使用 `event + payload` 结构，不要让不同模块随意定义完全不同的顶层字段。
- `event` 建议使用稳定的点分命名，例如 `math.add.completed`、`task.progress.updated`。
- `payload` 中只放客户端真正需要的数据，避免把内部实现细节直接暴露出去。
- 如果后续需要支持定向推送、分组订阅或多路由广播，建议在当前封装基础上继续扩展，而不是让业务层直接操作 WebSocket 底层连接。
