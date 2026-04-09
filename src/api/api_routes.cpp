#include "wmnext/api/api_routes.hpp"

#include "wmnext/api/root_api.hpp"
#include "wmnext/api/tts_api.hpp"

namespace wmnext::api {

void register_api_routes(
    httplib::Server& server,
    service::TtsService& tts_service,
    ws::WebSocketNotifier& websocket_notifier
) {
    register_root_api(server);
    register_tts_api(server, tts_service, websocket_notifier);
}

}  // namespace wmnext::api
