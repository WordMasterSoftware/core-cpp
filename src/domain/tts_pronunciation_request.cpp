#include "wmnext/domain/tts_pronunciation_request.hpp"

namespace wmnext::domain {

namespace {

[[nodiscard]] bool is_ascii_letter_or_separator(char character) {
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || character == '-'
        || character == '_';
}

}  // namespace

bool is_valid_language_code(std::string_view language) {
    if (language.size() < 2 || language.size() > 16) {
        return false;
    }

    return std::all_of(language.begin(), language.end(), is_ascii_letter_or_separator);
}

bool is_valid_tts_pronunciation_request(const TtsPronunciationRequest& request) {
    return !request.word.empty() && is_valid_language_code(request.language);
}

}  // namespace wmnext::domain
