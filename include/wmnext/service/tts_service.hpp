#pragma once

#include "wmnext/domain/tts_pronunciation_request.hpp"

namespace wmnext::service {

struct TtsPronunciationResult {
    std::string language;
    std::string word;
    std::string audio_url;
    std::string provider;
    bool cached;
    std::string message;
};

class TtsService {
public:
    virtual ~TtsService() = default;

    [[nodiscard]] virtual TtsPronunciationResult get_pronunciation(
        const domain::TtsPronunciationRequest& request
    ) const = 0;
};

class MockTtsService final : public TtsService {
public:
    [[nodiscard]] TtsPronunciationResult get_pronunciation(
        const domain::TtsPronunciationRequest& request
    ) const override;
};

}  // namespace wmnext::service
