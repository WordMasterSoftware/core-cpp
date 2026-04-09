#include "wmnext/api/root_api.hpp"

#include "wmnext/http/json_response.hpp"

#include <httplib.h>

namespace wmnext::api {

// 注册首页路由。
// 这里只提供项目级说明，不承载业务域信息，避免首页随着接口增长而膨胀。
void register_root_api(httplib::Server& server) {
    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        const http::Json body = {
            {"name", "WordMaster Next API"},
            {"status", "ok"},
            {"version", "v1"}
        };

        http::set_json_response(response, body);
    });
}

}  // namespace wmnext::api
