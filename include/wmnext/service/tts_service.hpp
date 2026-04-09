#pragma once

#include "wmnext/domain/tts_pronunciation_request.hpp"

namespace wmnext::service {

struct TtsPronunciationResult {
    bool status;
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
    explicit MockTtsService(std::string cache_directory);

    [[nodiscard]] TtsPronunciationResult get_pronunciation(
        const domain::TtsPronunciationRequest& request
    ) const override;

private:
    std::string cache_directory_;
};

}  // namespace wmnext::service
