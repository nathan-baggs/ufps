#include "audio/audio_manager.h"

#include <cstring>
#include <expected>
#include <ranges>
#include <vector>

#include <xaudio2.h>

#include "audio/track.h"
#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/string_map.h"

namespace
{

auto parse_chunk(ufps::DataBufferView source, ufps::DataBufferView control_code)
    -> std::expected<ufps::DataBufferView, std::string>
{
    const auto chunk_header = std::ranges::search(source, control_code);
    if (std::ranges::empty(chunk_header))
    {
        return std::unexpected("could not find control code in chunk");
    }

    const auto header_offset = std::ranges::distance(std::ranges::cbegin(source), std::ranges::cbegin(chunk_header));

    auto chunk_size = std::uint32_t{};
    std::memcpy(&chunk_size, source.data() + header_offset + chunk_header.size(), sizeof(chunk_size));

    ufps::log::debug("header_offset: {:x}", header_offset);
    ufps::log::debug("chunk_size: {:x}", chunk_size);

    return std::span{source.data() + header_offset + chunk_header.size() + sizeof(chunk_size), chunk_size};
}
}

namespace ufps
{

AudioManager::AudioManager(ResourceLoader &resource_loader)
    : voice_callback_{*this}
    , tracks_{}
    , xaudio_{}
    , mastering_voice_{}
    , voices_{}
{
    ensure(::XAudio2Create(std::out_ptr(xaudio_), 0, XAUDIO2_DEFAULT_PROCESSOR) == S_OK, "failed to create xaudio2");

    ensure(
        xaudio_->CreateMasteringVoice(
            std::out_ptr(mastering_voice_),
            XAUDIO2_DEFAULT_CHANNELS,
            XAUDIO2_DEFAULT_SAMPLERATE,
            0u,
            nullptr,
            nullptr) == S_OK,
        "failed to create mastering voice");

    const auto format = ::WAVEFORMATEX{
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = 1,
        .nSamplesPerSec = 48000,
        .nAvgBytesPerSec = 48000 * 2,
        .nBlockAlign = 2,
        .wBitsPerSample = 16,
        .cbSize = 0,
    };

    for (auto &[voice, is_free] : voices_)
    {
        ensure(
            xaudio_->CreateSourceVoice(
                std::out_ptr(voice), &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, std::addressof(voice_callback_)) == S_OK,
            "failed to create voice");
        is_free = true;
    }

    for (const auto &resource :
         resource_loader.resources("sounds") | std::views::filter([](const auto &e) { return e.ends_with(".wav"); }))
    {
        log::info("loading: {}", resource);
        const auto track_data = resource_loader.load_data_buffer(resource);

        const std::byte riff_cc[]{std::byte{'R'}, std::byte{'I'}, std::byte{'F'}, std::byte{'F'}};
        const auto riff_chunk = parse_chunk(track_data, riff_cc);
        if (!riff_chunk)
        {
            log::error("failed to parse wav: {} {}", resource, riff_chunk.error());
        }

        const std::byte wave_cc[]{std::byte{'W'}, std::byte{'A'}, std::byte{'V'}, std::byte{'E'}};
        const auto wave_chunk = parse_chunk(*riff_chunk, wave_cc);
        if (!wave_chunk)
        {
            log::error("failed to parse wav: {} {}", resource, wave_chunk.error());
        }

        const std::byte fmt_cc[]{std::byte{'f'}, std::byte{'m'}, std::byte{'t'}, std::byte{' '}};
        const auto fmt_chunk = parse_chunk(track_data, fmt_cc);
        if (!fmt_chunk)
        {
            log::error("failed to parse wav: {} {}", resource, fmt_chunk.error());
        }

        const std::byte data_cc[]{std::byte{'d'}, std::byte{'a'}, std::byte{'t'}, std::byte{'a'}};
        const auto data_chunk = parse_chunk(track_data, data_cc);
        if (!data_chunk)
        {
            log::error("failed to parse wav: {} {}", resource, data_chunk.error());
        }

        tracks_[resource.substr(resource.find_last_of('\\') + 1zu)] = Track{
            .format = {std::ranges::cbegin(*fmt_chunk), std::ranges::cend(*fmt_chunk)},
            .data = {std::ranges::cbegin(*data_chunk), std::ranges::cend(*data_chunk)},
        };
    }
}

auto AudioManager::play(std::string_view track_name, PlayMode mode) -> void
{
    const auto track = tracks_.find(track_name);
    if (track == std::ranges::cend(tracks_))
    {
        log::warn("tried to play missing track: {}", track_name);
        return;
    }

    auto next_voice = std::ranges::find(voices_, true, &Voice::is_free);

    if (next_voice != std::ranges::end(voices_))
    {
        auto &[voice, is_free] = *next_voice;
        is_free = false;

        const auto index = std::ranges::distance(std::ranges::begin(voices_), next_voice);
        void *index_as_void_ptr{};
        std::memcpy(&index_as_void_ptr, &index, sizeof(index));

        const auto buffer = ::XAUDIO2_BUFFER{
            .Flags = XAUDIO2_END_OF_STREAM,
            .AudioBytes = static_cast<::UINT32>(std::ranges::size(track->second.data)),
            .pAudioData = reinterpret_cast<const ::BYTE *>(std::ranges::data(track->second.data)),
            .PlayBegin = 0,
            .PlayLength = 0,
            .LoopBegin = 0,
            .LoopLength = 0,
            .LoopCount = mode == PlayMode::LOOP ? XAUDIO2_LOOP_INFINITE : 0u,
            .pContext = index_as_void_ptr,
        };

        ensure(voice->SubmitSourceBuffer(&buffer) == S_OK, "failed to set source");
        ensure(voice->Start(0) == S_OK, "failed to start sound");
    }
    else
    {
        log::warn("no free voice");
    }
}
}
