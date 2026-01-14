#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "graphics/opengl.h"
#include "graphics/texture.h"
#include "utils/auto_release.h"

namespace ufps
{

class FrameBuffer
{
  public:
    FrameBuffer(std::vector<const Texture *> colour_textures, const Texture *depth_texture, const std::string &name);

    auto native_handle() const -> ::GLuint;

    auto bind() const -> void;
    auto unbind() const -> void;

    auto width() const -> std::uint32_t;
    auto height() const -> std::uint32_t;
    auto colour_textures() const -> std::span<const Texture *const>;
    auto name() const -> std::string_view;

  private:
    AutoRelease<::GLuint> handle_;
    std::vector<const Texture *> colour_textures_;
    const Texture *depth_texture_;
    std::string name_;
};

}
