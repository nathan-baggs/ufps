#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

#include "graphics/colour.h"
#include "graphics/opengl.h"
#include "graphics/shader.h"
#include "maths/matrix4.h"
#include "utils/auto_release.h"

namespace ufps
{

class Program
{
  public:
    Program(const Shader &vertex_shader, const Shader &fragment_shader, std::string_view name)
        pre(vertex_shader.type() == ShaderType::VERTEX)      //
        pre(fragment_shader.type() == ShaderType::FRAGMENT); //

    Program(const Shader &compute_shader, std::string_view name) //
        pre(compute_shader.type() == ShaderType::COMPUTE);

    auto bind() -> void  //
        pre(!is_bound()) //
        post(is_bound());

    auto unbind() -> void //
        pre(is_bound())   //
        post(!is_bound());

    auto is_bound() const -> bool;

    auto native_handle() const -> ::GLuint;

    auto set_uniform(std::size_t index, std::uint32_t value) const -> void //
        pre(is_bound());

    auto set_uniform(std::size_t index, std::uint64_t value) const -> void //
        pre(is_bound());

    auto set_uniform(std::size_t index, float value) const -> void //
        pre(is_bound());

    auto set_uniform(std::size_t index, const Matrix4 &value) const -> void //
        pre(is_bound());

    auto set_uniform(std::size_t index, const Colour &value) const -> void //
        pre(is_bound());

    auto set_uniform(std::size_t index, const std::tuple<float, float> &value) const -> void //
        pre(is_bound());

    template <class... Args>
    auto set_uniforms(Args &&...args) const -> void //
        pre(is_bound())
    {
        [&]<std::size_t... Index>(std::index_sequence<Index...>)
        { (set_uniform(Index, std::forward<Args>(args)), ...); }(std::make_index_sequence<sizeof...(Args)>());
    }

  private:
    AutoRelease<::GLuint> handle_;
    bool is_bound_;
};

}
