#pragma once

namespace httplib {
class Server;
}

namespace wmnext::service {
class TtsService;
}

namespace wmnext::ws {
class WebSocketNotifier;
}

namespace wmnext::api {

void register_tts_api(
    httplib::Server& server,
    service::TtsService& tts_service,
    ws::WebSocketNotifier& websocket_notifier
);

}  // namespace wmnext::api
