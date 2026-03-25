#include "wmnext/app/server_application.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

// 程序主入口只负责最薄的一层调度：
// 创建应用对象、启动服务，并在最外层兜底处理未捕获异常。
// 这样 main.cpp 可以始终保持稳定简洁，不随着业务增长而膨胀。
int main() {
    try {
        wmnext::app::ServerApplication application;
        return application.run();
    } catch (const std::exception& ex) {
        // 输出标准异常信息，便于在终端快速定位启动失败原因。
        std::cerr << "Unhandled exception: " << ex.what() << '\n';
    } catch (...) {
        // 兜底处理未知异常，避免进程静默崩溃。
        std::cerr << "Unhandled unknown exception.\n";
    }

    return EXIT_FAILURE;
}
