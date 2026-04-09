#include "wmnext/service/tts_service.hpp"

#if defined(WMNEXT_WITH_FLITE)
#include <flite/flite.h>

extern "C" {
cst_voice* register_cmu_us_awb(void);
cst_voice* register_cmu_us_kal(void);
cst_voice* register_cmu_us_slt(void);
}
#endif

namespace wmnext::service {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr int kSampleRate = 16000;
constexpr double kCharacterDurationSeconds = 0.14;
constexpr double kSilenceDurationSeconds = 0.03;
constexpr std::uint16_t kBitsPerSample = 16;
constexpr std::uint16_t kChannelCount = 1;
constexpr std::int16_t kAmplitude = 9000;
constexpr std::string_view kAudioIndexFileName = "index.txt";
std::mutex g_audio_cache_mutex;

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

[[nodiscard]] std::string flite_voice_for_language(std::string_view language) {
    if (language == "en-GB") {
        return "awb";
    }

    if (language == "en" || language == "en-US") {
        return "slt";
    }

    return "kal";
}

[[nodiscard]] bool can_use_flite_for_language(std::string_view language) {
    return language == "en-US" || language == "en-GB" || language == "en";
}

#if defined(WMNEXT_WITH_FLITE)
[[nodiscard]] cst_voice* get_flite_voice(std::string_view language) {
    if (language == "en-GB") {
        return register_cmu_us_awb();
    }

    if (language == "en" || language == "en-US") {
        return register_cmu_us_slt();
    }

    return register_cmu_us_kal();
}

[[nodiscard]] bool generate_flite_audio(
    const std::filesystem::path& output_path,
    const domain::TtsPronunciationRequest& request
) {
    static std::once_flag init_flag;
    std::call_once(init_flag, [] {
        flite_init();
    });

    if (!can_use_flite_for_language(request.language)) {
        return false;
    }

    auto* voice = get_flite_voice(request.language);
    if (voice == nullptr) {
        return false;
    }

    cst_wave* wave = flite_text_to_wave(request.word.c_str(), voice);
    if (wave == nullptr) {
        return false;
    }

    cst_wave_save_riff(wave, output_path.string().c_str());
    delete_wave(wave);
    return std::filesystem::exists(output_path);
}
#else
[[nodiscard]] bool generate_flite_audio(
    const std::filesystem::path&,
    const domain::TtsPronunciationRequest&
) {
    return false;
}
#endif

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

[[nodiscard]] std::filesystem::path make_index_file_path(const std::filesystem::path& cache_directory) {
    return cache_directory / kAudioIndexFileName;
}

[[nodiscard]] std::unordered_map<std::string, std::string> load_audio_index(const std::filesystem::path& index_file_path) {
    std::unordered_map<std::string, std::string> index_entries;
    std::ifstream stream(index_file_path);
    if (!stream.is_open()) {
        return index_entries;
    }

    std::string line;
    while (std::getline(stream, line)) {
        const auto separator = line.find('\t');
        if (separator == std::string::npos) {
            continue;
        }

        const auto hash = line.substr(0, separator);
        const auto file_name = line.substr(separator + 1);
        if (!hash.empty() && !file_name.empty()) {
            index_entries[hash] = file_name;
        }
    }

    return index_entries;
}

[[nodiscard]] bool rewrite_audio_index(
    const std::filesystem::path& index_file_path,
    const std::unordered_map<std::string, std::string>& index_entries
) {
    std::ofstream stream(index_file_path, std::ios::trunc);
    if (!stream.is_open()) {
        return false;
    }

    for (const auto& [hash, file_name] : index_entries) {
        stream << hash << '\t' << file_name << '\n';
    }

    return stream.good();
}

[[nodiscard]] bool append_audio_index_entry(
    const std::filesystem::path& index_file_path,
    std::string_view hash,
    std::string_view file_name
) {
    std::ofstream stream(index_file_path, std::ios::app);
    if (!stream.is_open()) {
        return false;
    }

    stream << hash << '\t' << file_name << '\n';
    return stream.good();
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
    std::lock_guard lock(g_audio_cache_mutex);

    const auto cache_directory = std::filesystem::path(cache_directory_);
    std::error_code error_code;
    std::filesystem::create_directories(cache_directory, error_code);
    if (error_code) {
        return make_failure_result(request, "Failed to create audio cache directory.");
    }

    const auto index_file_path = make_index_file_path(cache_directory);
    auto index_entries = load_audio_index(index_file_path);
    const auto request_hash = hash_to_hex(fnv1a_hash(make_cache_key(request)));

    if (const auto index_it = index_entries.find(request_hash); index_it != index_entries.end()) {
        const auto file_path = cache_directory / index_it->second;
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

        index_entries.erase(index_it);
        if (!rewrite_audio_index(index_file_path, index_entries)) {
            return make_failure_result(request, "Failed to repair audio cache index.");
        }
    }

    const auto file_path = cache_directory / (request_hash + ".wav");
    if (!can_use_flite_for_language(request.language)) {
        return make_failure_result(request, "Embedded Flite currently supports only configured English voices.");
    }

    if (!generate_flite_audio(file_path, request)) {
        return make_failure_result(request, "Embedded Flite failed to generate pronunciation audio.");
    }

    if (!append_audio_index_entry(index_file_path, request_hash, file_path.filename().string())) {
        return make_failure_result(request, "Audio file generated but failed to update audio cache index.");
    }

    return TtsPronunciationResult{
        .status = true,
        .language = request.language,
        .word = request.word,
        .audio_url = make_audio_url(file_path),
        .provider = "flite-embedded",
        .cached = false,
        .message = "Embedded Flite pronunciation audio generated successfully."
    };
}

}  // namespace wmnext::service
