#pragma once

namespace wmnext::domain {

struct TtsPronunciationRequest {
    std::string language;
    std::string word;
};

[[nodiscard]] bool is_valid_language_code(std::string_view language);
[[nodiscard]] bool is_valid_tts_pronunciation_request(const TtsPronunciationRequest& request);

}  // namespace wmnext::domain
