#pragma once

#include <cstddef>
#include <string_view>

#include "src/utils/data_buffer.h"

namespace ufps
{

template <class T>
concept IsBuffer = requires(T t, DataBufferView data, std::size_t offset) {
    { t.write(data, offset) };
};

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
        , original_size_{size}
        , frame_offset_{}
    {
    }

    auto write(DataBufferView data, std::size_t offset) -> void
    {
        buffer_.write(data, offset + frame_offset_);
    }

    auto advance() -> void
    {
        frame_offset_ = (frame_offset_ + original_size_) % (original_size_ * Frames);
    }

    auto buffer() const -> const Buffer &
    {
        return buffer_;
    }

    auto original_size() const -> std::size_t
    {
        return original_size_;
    }

    auto frame_offset_bytes() const -> std::size_t
    {
        return frame_offset_;
    }

  private:
    Buffer buffer_;
    std::size_t original_size_;
    std::size_t frame_offset_;
};

}
