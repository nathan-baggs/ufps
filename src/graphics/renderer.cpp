#include "graphics/renderer.h"

#include <string_view>

#include "graphics/command_buffer.h"
#include "graphics/opengl.h"
#include "graphics/program.h"
#include "graphics/scene.h"
#include "graphics/shader.h"
#include "utils/auto_release.h"

using namespace std::literals;

namespace
{
constexpr auto sample_vertex_shader = R"(
#version 460 core
    
struct VertexData
{
    float position[3];
    float colour[3];
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

vec3 get_position(int index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_colour(int index)
{
    return vec3(
        data[index].colour[0],
        data[index].colour[1],
        data[index].colour[2]);
}

layout (location = 0) out vec3 out_colour;

void main()
{
    gl_Position = vec4(get_position(gl_VertexID), 1.0);
    out_colour = get_colour(gl_VertexID);
}
)"sv;

constexpr auto sample_fragment_shader = R"(
#version 460 core

layout(location = 0) in vec3 in_colour;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(in_colour, 1.0);
}
)"sv;

auto create_program() -> ufps::Program
{
    const auto sample_vert = ufps::Shader{sample_vertex_shader, ufps::ShaderType::VERTEX, "sample_vertex_shader"sv};
    const auto sample_frag =
        ufps::Shader{sample_fragment_shader, ufps::ShaderType::FRAGMENT, "sample_fragment_shader"sv};
    return ufps::Program{sample_vert, sample_frag, "sample_prog"sv};
}

}

namespace ufps
{

Renderer::Renderer()
    : dummy_vao_{0u, [](auto e) { ::glDeleteVertexArrays(1u, &e); }}
    , command_buffer_{}
    , program_{create_program()}
{
    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);

    program_.use();
}

auto Renderer::render(const Scene &scene) -> void
{
    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager.native_handle();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);

    const auto command_count = command_buffer_.build(scene);

    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer_.native_handle());

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(command_buffer_.offset_bytes()),
        command_count,
        0);

    command_buffer_.advance();
}

}
