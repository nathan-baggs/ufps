#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "graphics/opengl.h"
#include "utils/auto_release.h"
#include "utils/data_buffer.h"

namespace ufps
{

class Buffer
{
  public:
    Buffer(std::size_t size, std::string_view name);

    auto write(DataBufferView data, std::size_t offset) const -> void;

    auto native_handle() const -> ::GLuint;

    auto size() const -> std::size_t;

    auto name() const -> std::string_view;

    auto to_string() const -> std::string;

  private:
    AutoRelease<::GLuint> buffer_;
    std::size_t size_;
    std::string name_;
};

}
