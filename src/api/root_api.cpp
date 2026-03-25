#include "wmnext/api/root_api.hpp"

#include "wmnext/http/json_response.hpp"

#include <httplib.h>

namespace wmnext::api {

// 注册根路由。
// 它主要承担服务探活和说明作用，便于在浏览器或 curl 中快速确认服务是否正常启动。
void register_root_api(httplib::Server& server) {
    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        const http::Json body = {
            {"message", "C++ backend is running"},
            {"usage", "POST /add with JSON: {\"a\": 1, \"b\": 2}"}
        };

        // 所有 JSON 输出都走统一封装，避免在各个接口里重复设置 content-type。
        http::set_json_response(response, body);
    });
}

}  // namespace wmnext::api
