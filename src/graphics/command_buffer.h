#pragma once

#include <cstdint>
#include <string>

#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/scene.h"

namespace ufps
{

class CommandBuffer
{
  public:
    CommandBuffer();
    auto build(const Scene &scene) -> std::uint32_t;
    auto native_handle() const -> ::GLuint;
    auto advance() -> void;
    auto offset_bytes() const -> std::size_t;
    auto to_string() const -> std::string;

  private:
    MultiBuffer<PersistentBuffer> command_buffer_;
};

}
