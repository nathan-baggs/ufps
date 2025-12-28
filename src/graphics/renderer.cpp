#include "graphics/renderer.h"

#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/scene.h"
#include "graphics/command_buffer.h"
#include "graphics/object_data.h"
#include "graphics/opengl.h"
#include "graphics/program.h"
#include "graphics/shader.h"
#include "graphics/utils.h"
#include "utils/auto_release.h"

using namespace std::literals;

namespace
{
constexpr auto sample_vertex_shader = R"(
#version 460 core
    
struct VertexData
{
    float position[3];
    float uv[2];
};

struct ObjectData
{
    mat4 model;
    uint material_index;
};

struct MaterialData
{
    float colour[3];
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
};

layout(binding = 2, std430) readonly buffer objects {
    ObjectData object_data[];
};

layout(binding = 3, std430) readonly buffer materials {
    MaterialData material_data[];
};

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec2 get_uv(uint index)
{
    return vec2(
        data[index].uv[0],
        data[index].uv[1]);
}

layout (location = 0) out flat uint material_index;
layout (location = 1) out vec2 uv;

void main()
{
    gl_Position = projection * view * object_data[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
    material_index = object_data[gl_DrawID].material_index;
    uv = get_uv(gl_VertexID);
}
)"sv;

constexpr auto sample_fragment_shader = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

struct VertexData
{
    float position[3];
    float uv[2];
};

struct ObjectData
{
    mat4 model;
    uint material_index;
};

struct MaterialData
{
    float colour[3];
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

layout(binding = 1, std430) readonly buffer camera {
    mat4 view;
    mat4 projection;
};

layout(binding = 2, std430) readonly buffer objects {
    ObjectData object_data[];
};

layout(binding = 3, std430) readonly buffer materials {
    MaterialData material_data[];
};

vec3 get_colour(uint index)
{
    return vec3(
        material_data[index].colour[0],
        material_data[index].colour[1],
        material_data[index].colour[2]);
}

layout(location = 0, bindless_sampler) uniform sampler2D tex;

layout(location = 0) in flat uint material_index;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(get_colour(material_index) * texture(tex, uv).rgb, 1.0);
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
    , camera_buffer_{sizeof(CameraData), "camera_buffer"}
    , object_data_buffer_{sizeof(ObjectData), "object_data_buffer"}
    , program_{create_program()}
{
    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);

    program_.use();
}

auto Renderer::render(const Scene &scene) -> void
{
    camera_buffer_.write(scene.camera.data_view(), 0zu);

    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager.native_handle();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        1,
        camera_buffer_.native_handle(),
        camera_buffer_.frame_offset_bytes(),
        sizeof(CameraData));
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);

    const auto command_count = command_buffer_.build(scene);
    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer_.native_handle());

    const auto object_data = scene.entities |
                             std::views::transform(
                                 [&scene](const auto &e)
                                 {
                                     const auto index = scene.material_manager.index(e.material_key);
                                     return ObjectData{
                                         .model = e.transform,
                                         .material_id_index = index,
                                         .padding = {},
                                     };
                                 }) |
                             std::ranges::to<std::vector>();
    resize_gpu_buffer(object_data, object_data_buffer_, "object_data_buffer");
    object_data_buffer_.write(std::as_bytes(std::span{object_data.data(), object_data.size()}), 0zu);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, object_data_buffer_.native_handle());

    scene.material_manager.sync();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.material_manager.native_handle());

    ::glProgramUniformHandleui64ARB(program_.native_handle(), 0, scene.the_one_texture.native_handle());

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(command_buffer_.offset_bytes()),
        command_count,
        0);

    command_buffer_.advance();
    camera_buffer_.advance();
    object_data_buffer_.advance();
    scene.material_manager.advance();
}

}
