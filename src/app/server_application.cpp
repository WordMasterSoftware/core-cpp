#include "wmnext/app/server_application.hpp"

#include "wmnext/api/api_routes.hpp"
#include "wmnext/api/root_api.hpp"
#include "wmnext/service/tts_service.hpp"
#include "wmnext/ws/websocket_notifier.hpp"

#include <cstdlib>
#include <httplib.h>
#include <iostream>

namespace wmnext::app {

namespace {

// 当前服务监听端口。
// 如果后续需要从配置文件、环境变量或命令行读取端口，可以从这里演进出去。
constexpr int kPort = 8181;
constexpr int kWebSocketPort = 8182;
constexpr std::string_view kAudioCacheDirectory = "./audio_cache";
constexpr std::string_view kAudioCacheRoute = "/audio_cache";

}  // namespace

// 应用主运行流程：
// 1. 创建 HTTP Server 实例。
// 2. 注册各业务模块的路由。
// 3. 启动监听并返回退出码。
int ServerApplication::run() const {
    httplib::Server server;
    std::error_code error_code;
    std::filesystem::create_directories(kAudioCacheDirectory, error_code);
    if (error_code) {
        std::cerr << "Failed to prepare audio cache directory: " << error_code.message() << '\n';
        return EXIT_FAILURE;
    }

    if (!server.set_mount_point(std::string{kAudioCacheRoute}, std::string{kAudioCacheDirectory})) {
        std::cerr << "Failed to mount audio cache directory.\n";
        return EXIT_FAILURE;
    }

    service::MockTtsService tts_service(std::string{kAudioCacheDirectory});
    ws::WebSocketNotifier websocket_notifier;

    if (!websocket_notifier.start("0.0.0.0", kWebSocketPort)) {
        std::cerr << "Failed to start WebSocket server on port " << kWebSocketPort << '\n';
        return EXIT_FAILURE;
    }

    // HTTP 路由只在这里集中注册，后续新增领域模块时继续按模块扩展。
    api::register_api_routes(server, tts_service, websocket_notifier);

    std::cout << "Server listening on http://0.0.0.0:" << kPort << '\n';
    std::cout << "WebSocket listening on ws://0.0.0.0:" << kWebSocketPort << "/queue" << '\n';

    // listen 是阻塞调用；只有启动失败或服务结束时才会继续往下执行。
    if (!server.listen("0.0.0.0", kPort)) {
        std::cerr << "Failed to start server on port " << kPort << '\n';
        websocket_notifier.stop();
        return EXIT_FAILURE;
    }

    websocket_notifier.stop();
    return EXIT_SUCCESS;
}

}  // namespace wmnext::app
