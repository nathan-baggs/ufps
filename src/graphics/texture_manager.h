#pragma once

#include <cstdint>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/opengl.h"
#include "graphics/texture.h"

namespace ufps
{

class TextureManager
{
  public:
    TextureManager();

    auto add(Texture texture) -> std::uint32_t;
    auto add(std::vector<Texture> textures) -> std::uint32_t;

    auto native_handle() const -> ::GLuint;

  private:
    Buffer gpu_buffer_;
    std::vector<::GLuint64> cpu_buffer_;
    std::vector<Texture> textures_;
};

}
