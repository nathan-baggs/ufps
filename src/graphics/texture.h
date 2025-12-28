#pragma once

#include <string>

#include "graphics/opengl.h"
#include "graphics/sampler.h"
#include "graphics/texture_data.h"
#include "utils/auto_release.h"

namespace ufps
{

class Texture
{
  public:
    Texture(const TextureData &texture, const std::string &name, const Sampler &sampler);
    ~Texture();

    Texture(Texture &&) = default;
    auto operator=(Texture &&) -> Texture & = default;

    auto native_handle() const -> ::GLuint64;

    auto name() const -> std::string;

  private:
    AutoRelease<::GLuint> handle_;
    ::GLuint64 bindless_handle_;
    std::string name_;
};

}
