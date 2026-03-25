#pragma once

#include <nlohmann/json.hpp>
#include <string_view>

namespace httplib {
class Response;
}

namespace wmnext::http {

// 为 JSON 类型起一个项目内统一别名，避免业务层频繁直接暴露第三方命名空间。
using Json = nlohmann::json;

// 将 JSON 对象写入 HTTP 响应，并统一设置响应状态码与 content-type。
// 这样业务接口只关心返回什么数据，不需要每次都重复写序列化与响应头逻辑。
void set_json_response(httplib::Response& response, const Json& body, int status = 200);

// 构造统一的错误响应体。
// 后续如果错误结构要扩展，例如加入错误码、追踪 ID、字段级校验信息，
// 可以只在这里调整，避免各业务接口散落不同格式。
[[nodiscard]] Json make_error_response(std::string_view message);

}  // namespace wmnext::http
