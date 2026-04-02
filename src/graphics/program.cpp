#include "graphics/program.h"

#include <GL/gl.h>
#include <string_view>

#include "graphics/opengl.h"
#include "graphics/shader.h"
#include "third_party/opengl/glext.h"
#include "utils/error.h"

namespace
{

auto check_state(::GLuint handle, ::GLenum state, std::string_view name, std::string_view message) -> void
{
    auto res = ::GLint{};
    ::glGetProgramiv(handle, state, &res);
    if (res != GL_TRUE)
    {
        char log[512]{};
        ::glGetProgramInfoLog(handle, sizeof(log), nullptr, log);
        throw ufps::Exception("{}: {} {}", message, name, log);
    }
}

}

namespace ufps
{

Program::Program(const Shader &vertex_shader, const Shader &fragment_shader, std::string_view name)
    : handle_{}
    , is_bound_{false}
{
    expect(vertex_shader.type() == ShaderType::VERTEX, "shader is not a vertex shader");
    expect(fragment_shader.type() == ShaderType::FRAGMENT, "shader is not a fragment shader");

    handle_ = {::glCreateProgram(), ::glDeleteProgram};
    ensure(handle_, "failed to create opengl program");

    ::glObjectLabel(GL_PROGRAM, handle_, name.length(), name.data());

    ::glAttachShader(handle_, vertex_shader.native_handle());
    ::glAttachShader(handle_, fragment_shader.native_handle());
    ::glLinkProgram(handle_);
    ::glValidateProgram(handle_);

    check_state(handle_, GL_LINK_STATUS, name, "failed to link program");
    check_state(handle_, GL_VALIDATE_STATUS, name, "failed to validate program");
}

Program::Program(const Shader &compute_shader, std::string_view name)
    : handle_{}
    , is_bound_{false}
{
    expect(compute_shader.type() == ShaderType::COMPUTE, "shader is not a compute shader");

    handle_ = {::glCreateProgram(), ::glDeleteProgram};
    ensure(handle_, "failed to create opengl program");

    ::glObjectLabel(GL_PROGRAM, handle_, name.length(), name.data());

    ::glAttachShader(handle_, compute_shader.native_handle());
    ::glLinkProgram(handle_);
    ::glValidateProgram(handle_);

    check_state(handle_, GL_LINK_STATUS, name, "failed to link program");
    check_state(handle_, GL_VALIDATE_STATUS, name, "failed to validate program");
}

auto Program::bind() -> void
{
    expect(!is_bound_, "binding an already bound program");
    ::glUseProgram(handle_);
    is_bound_ = true;
}

auto Program::unbind() -> void
{
    expect(is_bound_, "unbinding an already unbound program");
    ::glUseProgram(0);
    is_bound_ = false;
}

auto Program::native_handle() const -> ::GLuint
{
    return handle_;
}

auto Program::set_uniform(std::size_t index, std::uint32_t value) const -> void
{
    expect(is_bound_, "setting uniform on unbound program");
    ::glProgramUniform1ui(handle_, index, value);
}

auto Program::set_uniform(std::size_t index, float value) const -> void
{
    expect(is_bound_, "setting uniform on unbound program");
    ::glProgramUniform1f(handle_, index, value);
}

auto Program::set_uniform(std::size_t index, const Matrix4 &value) const -> void
{
    expect(is_bound_, "setting uniform on unbound program");
    ::glProgramUniformMatrix4fv(handle_, index, 1u, GL_FALSE, value.data().data());
}

auto Program::set_uniform(std::size_t index, const Colour &value) const -> void
{
    expect(is_bound_, "setting uniform on unbound program");
    ::glProgramUniform3f(handle_, index, value.r, value.g, value.b);
}

}
