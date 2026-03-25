#include "wmnext/http/json_response.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

namespace wmnext::http {

// 统一设置 JSON 响应。
// 将序列化、状态码设置、content-type 声明封装在一处，减少业务代码重复。
void set_json_response(httplib::Response& response, const Json& body, int status) {
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

// 统一构建错误响应体。
// 保持所有接口的错误返回结构一致，是后续演进错误处理机制的基础。
Json make_error_response(std::string_view message) {
    return Json{
        {"error", message}
    };
}

}  // namespace wmnext::http
