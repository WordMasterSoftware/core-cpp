#pragma once

namespace wmnext::app {

// 应用启动入口对象。
// main 函数只负责创建它并调用 run()，这样后续可以把启动流程、配置加载、
// 日志初始化、路由装配等职责都集中放在 app 层，而不是堆积在 main.cpp。
class ServerApplication {
public:
    // 启动 HTTP 服务并返回进程退出码。
    [[nodiscard]] int run() const;
};

}  // namespace wmnext::app
