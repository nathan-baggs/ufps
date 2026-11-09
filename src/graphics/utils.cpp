#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "graphics/dds.h"
#include "graphics/texture.h"
#include "graphics/texture_data.h"
#include "graphics/utils.h"
#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/log.h"

namespace
{

auto channels_to_format(int num_channels, bool is_srgb) -> ufps::TextureFormat
{
    switch (num_channels)
    {
        using enum ufps::TextureFormat;

        case 1: return RED;
        case 3: return is_srgb ? SRGB : RGB;
        case 4: return is_srgb ? SRGBA : RGBA;
    }

    throw ufps::Exception("unsupported channel count: {}", num_channels);
}

auto to_native_format(::DXGI_FORMAT format) -> ufps::TextureFormat
{
    switch (format)
    {
        case DXGI_FORMAT_BC5_UNORM: return ufps::TextureFormat::BC5U;
        case DXGI_FORMAT_BC7_UNORM: return ufps::TextureFormat::BC7;
        case DXGI_FORMAT_BC7_UNORM_SRGB: return ufps::TextureFormat::BC7_SRGB;
        default: throw ufps::Exception("unsupported dxgi format: {}", std::to_underlying(format));
    }
}
}

namespace ufps
{
auto load_texture(DataBufferView image_data, bool is_srgb) -> TextureData
{
    auto width = int{};
    auto height = int{};
    auto num_channels = int{};

    static constexpr std::byte dds_magic[] = {
        static_cast<std::byte>(0x44),
        static_cast<std::byte>(0x44),
        static_cast<std::byte>(0x53),
        static_cast<std::byte>(0x20),
    };

    if (std::ranges::equal(dds_magic, image_data | std::views::take(sizeof(dds_magic))))
    {
        auto dds_header = DDS_HEADER{};
        std::memcpy(&dds_header, image_data.data() + sizeof(dds_magic), sizeof(dds_header));

        ensure(dds_header.dwSize == sizeof(dds_header), "invalid dds_header size: {}", dds_header.dwSize);
        if (dds_header.ddspf.dwFourCC == 0x30315844)
        {
            auto dx10_header = DDS_HEADER_DXT10{};
            std::memcpy(&dx10_header, image_data.data() + sizeof(dds_magic) + sizeof(dds_header), sizeof(dx10_header));

            return {
                .width = static_cast<std::uint32_t>(dds_header.dwWidth),
                .height = static_cast<std::uint32_t>(dds_header.dwHeight),
                .format = to_native_format(dx10_header.dxgiFormat),
                .data = image_data | std::views::drop(sizeof(dds_magic) + sizeof(dds_header) + sizeof(dx10_header)) |
                        std::ranges::to<std::vector>(),
                .is_compressed = true,
            };
        }
        else if (dds_header.ddspf.dwFourCC == 0x55354342)
        {
            return {
                .width = static_cast<std::uint32_t>(dds_header.dwWidth),
                .height = static_cast<std::uint32_t>(dds_header.dwHeight),
                .format = TextureFormat::BC5U,
                .data = image_data | std::views::drop(sizeof(dds_magic) + sizeof(dds_header)) |
                        std::ranges::to<std::vector>(),
                .is_compressed = true,
            };
        }
        else
        {
            throw ufps::Exception("unsupported dds format: {}", dds_header.ddspf.dwFourCC);
        }
    }
    else
    {
        log::warn("handling png");

        auto raw_data = std::unique_ptr<::stbi_uc, void (*)(void *)>{
            ::stbi_load_from_memory(
                reinterpret_cast<const ::stbi_uc *>(image_data.data()),
                image_data.size(),
                &width,
                &height,
                &num_channels,
                0),
            ::stbi_image_free};
        ensure(raw_data, "failed to parse texture data");

        const auto *ptr = reinterpret_cast<const std::byte *>(raw_data.get());

        return {
            .width = static_cast<std::uint32_t>(width),
            .height = static_cast<std::uint32_t>(height),
            .format = channels_to_format(num_channels, is_srgb),
            .data = {{ptr, ptr + width * height * num_channels}},
            .is_compressed = false,
        };
    }
}

}
