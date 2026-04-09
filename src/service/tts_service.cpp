#include "wmnext/service/tts_service.hpp"

namespace wmnext::service {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kSampleRate = 16000;
constexpr double kCharacterDurationSeconds = 0.14;
constexpr double kSilenceDurationSeconds = 0.03;
constexpr std::uint16_t kBitsPerSample = 16;
constexpr std::uint16_t kChannelCount = 1;
constexpr std::int16_t kAmplitude = 9000;

[[nodiscard]] std::uint64_t fnv1a_hash(std::string_view value) {
    constexpr std::uint64_t kOffsetBasis = 14695981039346656037ULL;
    constexpr std::uint64_t kPrime = 1099511628211ULL;

    std::uint64_t hash = kOffsetBasis;
    for (const auto character : value) {
        hash ^= static_cast<unsigned char>(character);
        hash *= kPrime;
    }

    return hash;
}

[[nodiscard]] std::string hash_to_hex(std::uint64_t hash) {
    std::ostringstream stream;
    stream << std::hex << std::nouppercase << std::setfill('0') << std::setw(16) << hash;
    return stream.str();
}

void write_little_endian_16(std::ostream& stream, std::uint16_t value) {
    stream.put(static_cast<char>(value & 0xFF));
    stream.put(static_cast<char>((value >> 8) & 0xFF));
}

void write_little_endian_32(std::ostream& stream, std::uint32_t value) {
    stream.put(static_cast<char>(value & 0xFF));
    stream.put(static_cast<char>((value >> 8) & 0xFF));
    stream.put(static_cast<char>((value >> 16) & 0xFF));
    stream.put(static_cast<char>((value >> 24) & 0xFF));
}

[[nodiscard]] double character_frequency(char character) {
    const auto normalized = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(character)));
    if (normalized >= 'a' && normalized <= 'z') {
        return 440.0 + static_cast<double>(normalized - 'a') * 18.0;
    }

    if (normalized >= '0' && normalized <= '9') {
        return 700.0 + static_cast<double>(normalized - '0') * 12.0;
    }

    return 380.0;
}

[[nodiscard]] std::vector<std::int16_t> build_mock_audio_samples(std::string_view word) {
    const auto tone_samples = static_cast<std::size_t>(kSampleRate * kCharacterDurationSeconds);
    const auto silence_samples = static_cast<std::size_t>(kSampleRate * kSilenceDurationSeconds);

    std::vector<std::int16_t> samples;
    samples.reserve(std::max<std::size_t>(1, word.size()) * (tone_samples + silence_samples));

    for (const auto character : word) {
        const auto frequency = character_frequency(character);

        for (std::size_t index = 0; index < tone_samples; ++index) {
            const auto angle = 2.0 * kPi * frequency * static_cast<double>(index) / static_cast<double>(kSampleRate);
            const auto sample = static_cast<std::int16_t>(std::sin(angle) * static_cast<double>(kAmplitude));
            samples.push_back(sample);
        }

        samples.insert(samples.end(), silence_samples, 0);
    }

    if (samples.empty()) {
        samples.insert(samples.end(), kSampleRate / 4, 0);
    }

    return samples;
}

[[nodiscard]] bool write_wav_file(const std::filesystem::path& output_path, const std::vector<std::int16_t>& samples) {
    std::ofstream stream(output_path, std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        return false;
    }

    const std::uint32_t data_size = static_cast<std::uint32_t>(samples.size() * sizeof(std::int16_t));
    const std::uint32_t riff_chunk_size = 36 + data_size;
    const std::uint32_t byte_rate = kSampleRate * kChannelCount * (kBitsPerSample / 8);
    const std::uint16_t block_align = kChannelCount * (kBitsPerSample / 8);

    stream.write("RIFF", 4);
    write_little_endian_32(stream, riff_chunk_size);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    write_little_endian_32(stream, 16);
    write_little_endian_16(stream, 1);
    write_little_endian_16(stream, kChannelCount);
    write_little_endian_32(stream, kSampleRate);
    write_little_endian_32(stream, byte_rate);
    write_little_endian_16(stream, block_align);
    write_little_endian_16(stream, kBitsPerSample);
    stream.write("data", 4);
    write_little_endian_32(stream, data_size);

    for (const auto sample : samples) {
        write_little_endian_16(stream, static_cast<std::uint16_t>(sample));
    }

    return stream.good();
}

[[nodiscard]] std::string make_cache_key(const domain::TtsPronunciationRequest& request) {
    return request.language + "\n" + request.word;
}

[[nodiscard]] std::filesystem::path make_audio_cache_file_path(
    const std::filesystem::path& cache_directory,
    const domain::TtsPronunciationRequest& request
) {
    return cache_directory / (hash_to_hex(fnv1a_hash(make_cache_key(request))) + ".wav");
}

[[nodiscard]] std::string make_audio_url(const std::filesystem::path& file_path) {
    return "/audio_cache/" + file_path.filename().string();
}

[[nodiscard]] TtsPronunciationResult make_failure_result(
    const domain::TtsPronunciationRequest& request,
    std::string message
) {
    return TtsPronunciationResult{
        .status = false,
        .language = request.language,
        .word = request.word,
        .audio_url = "",
        .provider = "mock-tts",
        .cached = false,
        .message = std::move(message)
    };
}

}  // namespace

MockTtsService::MockTtsService(std::string cache_directory)
    : cache_directory_(std::move(cache_directory)) {
}

TtsPronunciationResult MockTtsService::get_pronunciation(const domain::TtsPronunciationRequest& request) const {
    const auto cache_directory = std::filesystem::path(cache_directory_);
    std::error_code error_code;
    std::filesystem::create_directories(cache_directory, error_code);
    if (error_code) {
        return make_failure_result(request, "Failed to create audio cache directory.");
    }

    const auto file_path = make_audio_cache_file_path(cache_directory, request);

    if (std::filesystem::exists(file_path, error_code) && !error_code) {
        return TtsPronunciationResult{
            .status = true,
            .language = request.language,
            .word = request.word,
            .audio_url = make_audio_url(file_path),
            .provider = "mock-tts",
            .cached = true,
            .message = "Mock pronunciation audio served from cache."
        };
    }

    if (error_code) {
        return make_failure_result(request, "Failed to access audio cache.");
    }

    const auto samples = build_mock_audio_samples(request.word);
    if (!write_wav_file(file_path, samples)) {
        return make_failure_result(request, "Failed to generate mock pronunciation audio.");
    }

    return TtsPronunciationResult{
        .status = true,
        .language = request.language,
        .word = request.word,
        .audio_url = make_audio_url(file_path),
        .provider = "mock-tts",
        .cached = false,
        .message = "Mock pronunciation metadata generated successfully."
    };
}

}  // namespace wmnext::service
