#pragma once

#include <cstddef>
#include <string_view>

#include "graphics/utils.h"
#include "utils/data_buffer.h"

namespace ufps
{

/**
 * A multi buffer wrapper over a Buffer type. Will allocate size * Frames amount of data and can advance through the
 * internal frames
 */
template <IsBuffer Buffer, std::size_t Frames = 3zu>
class MultiBuffer
{
  public:
    MultiBuffer(std::size_t size, std::string_view name)
        : buffer_{size * Frames, name}
        , size_{size}
        , frame_offset_{}
    {
    }

    auto write(DataBufferView data, std::size_t offset) -> void
    {
        buffer_.write(data, offset + frame_offset_);
    }

    auto advance() -> void
    {
        frame_offset_ = (frame_offset_ + size_) % (size_ * Frames);
    }

    auto native_handle() const
    {
        return buffer_.native_handle();
    }

    auto buffer() const -> const Buffer &
    {
        return buffer_;
    }

    auto size() const -> std::size_t
    {
        return size_;
    }

    auto frame_offset_bytes() const -> std::size_t
    {
        return frame_offset_;
    }

  private:
    Buffer buffer_;
    std::size_t size_;
    std::size_t frame_offset_;
};

}
