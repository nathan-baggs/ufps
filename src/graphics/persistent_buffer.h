#pragma once

#include <cstddef>
#include <string_view>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/data_buffer.h"

namespace ufps
{

class PersistentBuffer
{
  public:
    PersistentBuffer(std::size_t size, std::string_view name);

    auto write(DataBufferView data, std::size_t offset) const -> void;

    auto native_handle() const -> ::GLuint;

  private:
    AutoRelease<::GLuint> buffer_;
    std::size_t size_;
    void *map_;
};

}
