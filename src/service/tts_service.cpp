#include "wmnext/service/tts_service.hpp"

namespace wmnext::service {

namespace {

[[nodiscard]] std::string sanitize_for_url(std::string value) {
    std::replace(value.begin(), value.end(), ' ', '-');

    for (auto& character : value) {
        if (!(std::isalnum(static_cast<unsigned char>(character)) || character == '-' || character == '_' || character == '.')) {
            character = '-';
        }
    }

    return value;
}

}  // namespace

TtsPronunciationResult MockTtsService::get_pronunciation(const domain::TtsPronunciationRequest& request) const {
    const auto normalized_language = sanitize_for_url(request.language);
    const auto normalized_word = sanitize_for_url(request.word);

    return TtsPronunciationResult{
        .language = request.language,
        .word = request.word,
        .audio_url = "https://tts.wordmaster.next/v1/audio/" + normalized_language + "/" + normalized_word + ".mp3",
        .provider = "mock-tts",
        .cached = false,
        .message = "Mock pronunciation metadata generated successfully."
    };
}

}  // namespace wmnext::service
