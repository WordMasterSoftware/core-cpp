#include "wmnext/api/tts_api.hpp"

#include "wmnext/http/json_response.hpp"
#include "wmnext/service/tts_service.hpp"
#include "wmnext/ws/websocket_notifier.hpp"

#include <httplib.h>

namespace wmnext::api {

namespace {

[[nodiscard]] std::string trim_copy(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

}  // namespace

void register_tts_api(
    httplib::Server& server,
    service::TtsService& tts_service,
    ws::WebSocketNotifier& websocket_notifier
) {
    server.Get("/api/tts", [&tts_service, &websocket_notifier](const httplib::Request& request, httplib::Response& response) {
        const auto raw_language = request.has_param("language") ? request.get_param_value("language") : std::string{};
        const auto raw_word = request.has_param("word") ? request.get_param_value("word") : std::string{};

        domain::TtsPronunciationRequest pronunciation_request{
            .language = trim_copy(raw_language),
            .word = trim_copy(raw_word)
        };

        if (!domain::is_valid_tts_pronunciation_request(pronunciation_request)) {
            http::set_json_response(
                response,
                http::make_error_response("Query parameters 'language' and 'word' are required, and language must be a valid code."),
                400
            );
            return;
        }

        const auto result = tts_service.get_pronunciation(pronunciation_request);
        const http::Json response_body = {
            {"language", result.language},
            {"word", result.word},
            {"audio_url", result.audio_url},
            {"provider", result.provider},
            {"cached", result.cached},
            {"message", result.message}
        };

        http::set_json_response(response, response_body);

        websocket_notifier.notify(
            "tts.pronunciation.requested",
            http::Json{
                {"language", result.language},
                {"word", result.word},
                {"provider", result.provider}
            }
        );
    });
}

}  // namespace wmnext::api
