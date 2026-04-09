#pragma once

namespace httplib {
class Server;
}

namespace wmnext::api {

// 注册首页接口。
// 根路径只提供最小化服务说明，避免和业务接口混在一起。
void register_root_api(httplib::Server& server);

}  // namespace wmnext::api
