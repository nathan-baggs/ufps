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

auto Program::use() const -> void
{
    ::glUseProgram(handle_);
}

}
