#pragma once

namespace httplib {
class Server;
}

namespace wmnext::api {

// 注册根路径接口。
// 这里通常用于健康检查、欢迎信息或服务说明，是最基础的入口路由。
void register_root_api(httplib::Server& server);

}  // namespace wmnext::api
