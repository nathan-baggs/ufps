#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/data_buffer.h"

namespace ufps
{

class Buffer
{
  public:
    Buffer(std::uint32_t size);

    auto write(DataBufferView data, std::size_t offset) const -> void;

    auto native_handle() const -> ::GLuint;

  private:
    AutoRelease<::GLuint> buffer_;
    std::uint32_t size_;
};

}
