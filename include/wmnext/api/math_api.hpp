#pragma once

namespace httplib {
class Server;
}

namespace wmnext::ws {
class WebSocketNotifier;
}

namespace wmnext::api {

// 注册数学相关接口。
// 当前仅包含 /add，后续如果增加减乘除或更复杂计算接口，也继续归在这一模块中。
void register_math_api(httplib::Server& server, ws::WebSocketNotifier& websocket_notifier);

}  // namespace wmnext::api
