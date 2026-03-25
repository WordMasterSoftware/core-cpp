#include "wmnext/app/server_application.hpp"

#include "wmnext/api/math_api.hpp"
#include "wmnext/api/root_api.hpp"

#include <cstdlib>
#include <httplib.h>
#include <iostream>

namespace wmnext::app {

namespace {

// 当前服务监听端口。
// 如果后续需要从配置文件、环境变量或命令行读取端口，可以从这里演进出去。
constexpr int kPort = 8080;

}  // namespace

// 应用主运行流程：
// 1. 创建 HTTP Server 实例。
// 2. 注册各业务模块的路由。
// 3. 启动监听并返回退出码。
int ServerApplication::run() const {
    httplib::Server server;

    // 在这里集中装配所有 API 模块，方便未来继续扩展更多业务域。
    api::register_root_api(server);
    api::register_math_api(server);

    std::cout << "Server listening on http://0.0.0.0:" << kPort << '\n';

    // listen 是阻塞调用；只有启动失败或服务结束时才会继续往下执行。
    if (!server.listen("0.0.0.0", kPort)) {
        std::cerr << "Failed to start server on port " << kPort << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

}  // namespace wmnext::app
