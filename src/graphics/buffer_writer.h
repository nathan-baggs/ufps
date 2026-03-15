#pragma once

#include <cstddef>
#include <span>

#include "graphics/utils.h"

namespace ufps
{

template <IsBuffer Buffer>
class BufferWriter
{
  public:
    constexpr BufferWriter(Buffer &buffer)
        : buffer_(buffer)
        , offset_{}
    {
    }

    template <class T>
    auto write(const T &obj) -> void
        requires(std::is_trivially_copyable_v<T>)
    {
        auto spn = std::span<const T>{&obj, 1};
        write(spn);
    }

    template <class T>
    auto write(std::span<const T> data) -> void
    {
        buffer_.write(std::as_bytes(data), offset_);
        offset_ += data.size_bytes();
    }

  private:
    Buffer &buffer_;

    std::size_t offset_;
};

}
