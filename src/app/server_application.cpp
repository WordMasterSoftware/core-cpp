#include "wmnext/app/server_application.hpp"

#include "wmnext/api/math_api.hpp"
#include "wmnext/api/root_api.hpp"
#include "wmnext/mq/message_queue.hpp"
#include "wmnext/ws/websocket_message_queue_server.hpp"

#include <cstdlib>
#include <httplib.h>
#include <iostream>

namespace wmnext::app {

namespace {

// 当前服务监听端口。
// 如果后续需要从配置文件、环境变量或命令行读取端口，可以从这里演进出去。
constexpr int kPort = 8181;
constexpr int kWebSocketPort = 8182;

}  // namespace

// 应用主运行流程：
// 1. 创建 HTTP Server 实例。
// 2. 注册各业务模块的路由。
// 3. 启动监听并返回退出码。
int ServerApplication::run() const {
    httplib::Server server;
    mq::MessageQueue message_queue;
    ws::WebSocketMessageQueueServer websocket_server;

    if (!websocket_server.start("0.0.0.0", kWebSocketPort, message_queue)) {
        std::cerr << "Failed to start WebSocket server on port " << kWebSocketPort << '\n';
        return EXIT_FAILURE;
    }

    // 在这里集中装配所有 API 模块，方便未来继续扩展更多业务域。
    api::register_root_api(server);
    api::register_math_api(server, message_queue);

    std::cout << "Server listening on http://0.0.0.0:" << kPort << '\n';
    std::cout << "WebSocket queue listening on ws://0.0.0.0:" << kWebSocketPort << "/queue" << '\n';

    // listen 是阻塞调用；只有启动失败或服务结束时才会继续往下执行。
    if (!server.listen("0.0.0.0", kPort)) {
        std::cerr << "Failed to start server on port " << kPort << '\n';
        message_queue.shutdown();
        websocket_server.stop();
        return EXIT_FAILURE;
    }

    message_queue.shutdown();
    websocket_server.stop();
    return EXIT_SUCCESS;
}

}  // namespace wmnext::app
