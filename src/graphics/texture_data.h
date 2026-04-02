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
    RGB16F,
    DEPTH24,
    BC7,
};

struct TextureData
{
    std::uint32_t width;
    std::uint32_t height;
    TextureFormat format;
    std::optional<DataBuffer> data;
    bool is_compressed;
};

inline auto to_string(TextureFormat format) -> std::string
{
    switch (format)
    {
        using enum TextureFormat;
        case RED: return "RED";
        case RGB: return "RGB";
        case SRGB: return "SRGB";
        case RGBA: return "RGBA";
        case SRGBA: return "SRGBA";
        case RGB16F: return "RGB16F";
        case DEPTH24: return "DEPTH24";
        case BC7: return "BC7";
        default: return "<UNKNOWN>";
    }
}

}
