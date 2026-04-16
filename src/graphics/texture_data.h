#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "utils/data_buffer.h"

namespace ufps
{

enum class TextureFormat
{
    RED,
    RGB,
    SRGB,
    RGBA,
    SRGBA,
    R16F,
    RGB16F,
    DEPTH24,
    BC5U,
    BC7,
    BC7_SRGB,
};

struct TextureData
{
    std::uint32_t width;
    std::uint32_t height;
    TextureFormat format;
    std::optional<DataBuffer> data;
    bool is_compressed;
};

}
