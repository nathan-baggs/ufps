#pragma once

#include <cstdint>
#include <string>

#include "core/scene.h"
#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"

namespace ufps
{

class CommandBuffer
{
  public:
    CommandBuffer();
    auto build(const Scene &scene) -> std::uint32_t;
    auto build(const Entity &entity) -> std::uint32_t;
    auto native_handle() const -> ::GLuint;
    auto advance() -> void;
    auto offset_bytes() const -> std::size_t;
    auto to_string() const -> std::string;
    auto name() const -> std::string_view;

  private:
    MultiBuffer<PersistentBuffer> command_buffer_;
};

}
