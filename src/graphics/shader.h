#pragma once

#include <string>
#include <string_view>

#include "graphics/opengl.h"
#include "utils/auto_release.h"

namespace ufps
{

enum class ShaderType
{
    VERTEX,
    FRAGMENT
};

class Shader
{
  public:
    Shader(std::string_view source, ShaderType type, std::string_view name);

    auto type() const -> ShaderType;
    auto native_handle() const -> ::GLuint;

  private:
    AutoRelease<::GLuint> handle_;
    ShaderType type_;
};

auto to_string(ShaderType obj) -> std::string;

}
