#include "graphics/renderer.h"

#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/scene.h"
#include "events/key.h"
#include "graphics/command_buffer.h"
#include "graphics/object_data.h"
#include "graphics/opengl.h"
#include "graphics/point_light.h"
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
    float normal[3];
    float tangent[3];
    float bitangent[3];
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

layout(binding = 4, std430) readonly buffer lights {
    float ambient_colour[3];
    float point_light_pos[3];
    float point_light_colour[3];
    float point_light_attenuation[3];
};

vec3 get_position(uint index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_normal(uint index)
{
    return vec3(
        data[index].normal[0],
        data[index].normal[1],
        data[index].normal[2]);
}

vec3 get_tangent(uint index)
{
    return vec3(
        data[index].tangent[0],
        data[index].tangent[1],
        data[index].tangent[2]);
}

vec3 get_bitangent(uint index)
{
    return vec3(
        data[index].bitangent[0],
        data[index].bitangent[1],
        data[index].bitangent[2]);
}

vec2 get_uv(uint index)
{
    return vec2(
        data[index].uv[0],
        data[index].uv[1]);
}

layout (location = 0) out flat uint out_material_index;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out vec4 out_frag_position;
layout (location = 3) out mat3 out_tbn;

void main()
{
    mat3 normal_mat = transpose(inverse(mat3(object_data[gl_DrawID].model)));

    out_frag_position = object_data[gl_DrawID].model * vec4(get_position(gl_VertexID), 1.0);
    gl_Position = projection * view * out_frag_position;
    out_material_index = object_data[gl_DrawID].material_index;
    out_uv = get_uv(gl_VertexID);

    vec3 t = normalize(vec3(object_data[gl_DrawID].model * vec4(get_tangent(gl_VertexID), 0.0)));
    vec3 b = normalize(vec3(object_data[gl_DrawID].model * vec4(get_bitangent(gl_VertexID), 0.0)));
    vec3 n = normalize(vec3(object_data[gl_DrawID].model * vec4(get_normal(gl_VertexID), 0.0)));
    out_tbn = mat3(t, b, n);
}
)"sv;

constexpr auto sample_fragment_shader = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

struct VertexData
{
    float position[3];
    float normal[3];
    float tangent[3];
    float bitangent[3];
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

layout(binding = 4, std430) readonly buffer lights {
    float ambient_colour[3];
    float point_light_pos[3];
    float point_light_colour[3];
    float point_light_attenuation[3];
};

vec3 get_colour(uint index)
{
    return vec3(
        material_data[index].colour[0],
        material_data[index].colour[1],
        material_data[index].colour[2]);
}

vec3 calc_point(vec3 frag_position, vec3 n)
{
    vec3 pos = vec3(point_light_pos[0], point_light_pos[1], point_light_pos[2]);
    vec3 colour = vec3(point_light_colour[0], point_light_colour[1], point_light_colour[2]);
    vec3 attenuation = vec3(point_light_attenuation[0], point_light_attenuation[1], point_light_attenuation[2]);

    float distance = length(pos - frag_position);
    float att = 1.0 / (attenuation.x + (attenuation.y * distance) + (attenuation.z * (distance * distance)));

    vec3 light_dir = normalize(pos - frag_position);
    float diff = max(dot(n, light_dir), 0.0);

    return diff * att * colour;
}

layout(location = 0, bindless_sampler) uniform sampler2D albedo_tex;
layout(location = 1, bindless_sampler) uniform sampler2D normal_tex;

layout(location = 0) in flat uint in_material_index;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_frag_position;
layout(location = 3) in mat3 in_tbn;

layout(location = 0) out vec4 out_colour;

void main()
{
    vec3 n = texture(normal_tex, in_uv).xyz;
    n = (n * 2.0) - 1.0;
    n = normalize(in_tbn * n);

    vec3 albedo = texture(albedo_tex, in_uv).rgb;
    vec3 amb_colour = vec3(ambient_colour[0], ambient_colour[1], ambient_colour[2]);
    vec3 point_colour = calc_point(in_frag_position.xyz, n);

    out_colour = vec4(albedo * (amb_colour + point_colour), 1.0);
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
    , light_buffer_{sizeof(LightData), "light_buffer"}
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

    light_buffer_.write(std::as_bytes(std::span<const LightData, 1zu>{&scene.lights, 1zu}), 0zu);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, light_buffer_.native_handle());

    ::glProgramUniformHandleui64ARB(program_.native_handle(), 0, scene.the_one_texture.native_handle());
    ::glProgramUniformHandleui64ARB(program_.native_handle(), 1, scene.the_one_normal.native_handle());

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(command_buffer_.offset_bytes()),
        command_count,
        0);

    command_buffer_.advance();
    camera_buffer_.advance();
    light_buffer_.advance();
    object_data_buffer_.advance();
    scene.material_manager.advance();
}

}
