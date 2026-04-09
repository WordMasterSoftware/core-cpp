#include "wmnext/api/math_api.hpp"

#include "wmnext/http/json_response.hpp"
#include "wmnext/ws/websocket_notifier.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

namespace wmnext::api {

namespace {

// 校验请求体中是否存在可参与加法运算的两个数字字段。
// 这类输入校验独立成辅助函数后，路由处理逻辑会更直观，后续也更容易复用。
bool has_valid_operands(const http::Json& request_body) {
    return request_body.contains("a")
        && request_body.contains("b")
        && request_body["a"].is_number()
        && request_body["b"].is_number();
}

}  // namespace

// 注册数学接口。
// 这里采用“注册函数 + lambda 处理器”的形式，便于把不同业务域拆到不同文件中维护。
void register_math_api(httplib::Server& server, ws::WebSocketNotifier& websocket_notifier) {
    server.Post("/add", [&websocket_notifier](const httplib::Request& request, httplib::Response& response) {
        try {
            // 先将请求体解析为 JSON；如果格式非法，会进入下方 parse_error 分支。
            const http::Json request_body = http::Json::parse(request.body);

            if (!has_valid_operands(request_body)) {
                http::set_json_response(
                    response,
                    http::make_error_response("Request JSON must contain numeric fields 'a' and 'b'."),
                    400
                );
                return;
            }

            // 通过统一校验后再提取数值，保证 get<double>() 的调用足够安全、语义清晰。
            const double a = request_body["a"].get<double>();
            const double b = request_body["b"].get<double>();

            const double result = a + b;
            const http::Json response_body = {
                {"result", result}
            };

            http::set_json_response(response, response_body);

            websocket_notifier.notify(
                "math.add.completed",
                http::Json{
                    {"a", a},
                    {"b", b},
                    {"result", result}
                }
            );
        } catch (const nlohmann::json::parse_error&) {
            // 统一返回 JSON 错误结构，方便前端或调用方稳定处理错误。
            http::set_json_response(response, http::make_error_response("Invalid JSON body."), 400);
        }
    });
}

}  // namespace wmnext::api
