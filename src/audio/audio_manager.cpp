#include "audio/audio_manager.h"

#include <cstring>
#include <expected>
#include <ranges>
#include <vector>

#include "audio/track.h"
#include "resources/resource_loader.h"
#include "utils/data_buffer.h"
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
    : tracks_{}
{
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

        tracks_[resource.substr(resource.find_last_of('\\'))] = Track{
            .format = {std::ranges::cbegin(*fmt_chunk), std::ranges::cend(*fmt_chunk)},
            .data = {std::ranges::cbegin(*data_chunk), std::ranges::cend(*data_chunk)},
        };
    }
}
}
