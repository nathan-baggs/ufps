#include "graphics/renderer.h"

#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/entity.h"
#include "core/scene.h"
#include "graphics/command_buffer.h"
#include "graphics/frame_buffer.h"
#include "graphics/mesh_manager.h"
#include "graphics/object_data.h"
#include "graphics/opengl.h"
#include "graphics/point_light.h"
#include "graphics/program.h"
#include "graphics/sampler.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/texture_data.h"
#include "graphics/texture_manager.h"
#include "graphics/utils.h"
#include "resources/resource_loader.h"
#include "third_party/opengl/glext.h"
#include "utils/auto_release.h"

using namespace std::literals;

namespace
{
auto create_program(
    ufps::ResourceLoader &resource_loader,
    std::string_view vertex_path,
    std::string_view vertex_name,
    std::string_view fragment_path,
    std::string_view fragment_name,
    std::string_view program_name) -> ufps::Program
{
    const auto sample_vert =
        ufps::Shader{resource_loader.load_string(vertex_path), ufps::ShaderType::VERTEX, vertex_name};
    const auto sample_frag =
        ufps::Shader{resource_loader.load_string(fragment_path), ufps::ShaderType::FRAGMENT, fragment_name};
    return ufps::Program{sample_vert, sample_frag, program_name};
}

auto create_render_target(
    std::uint32_t colour_attachment_count,
    std::uint32_t width,
    std::uint32_t height,
    ufps::Sampler &sampler,
    ufps::TextureManager &texture_manager,
    std::string_view name) -> ufps::RenderTarget
{
    const auto colour_attachment_texture_data = ufps::TextureData{
        .width = width,
        .height = height,
        .format = ufps::TextureFormat::RGB16F,
        .data = std::nullopt,
    };

    auto colour_attachements =
        std::views::iota(0u, colour_attachment_count) |
        std::views::transform(
            [&](auto index)
            {
                return ufps::Texture{
                    colour_attachment_texture_data, std::format("{}_{}_texture", name, index), sampler};
            }) |
        std::ranges::to<std::vector>();

    const auto first_index = texture_manager.add(std::move(colour_attachements));

    const auto depth_texture_data = ufps::TextureData{
        .width = width,
        .height = height,
        .format = ufps::TextureFormat::DEPTH24,
        .data = std::nullopt,
    };
    auto depth_texture = ufps::Texture{depth_texture_data, std::format("{}_depth_texture", name), sampler};
    const auto depth_texture_index = texture_manager.add(std::move(depth_texture));

    auto fb = ufps::FrameBuffer{
        texture_manager.textures(
            std::views::iota(first_index, first_index + colour_attachment_count) | std::ranges::to<std::vector>()),
        texture_manager.texture(depth_texture_index),
        std::format("{}_frame_buffer", name)};

    return {
        .fb = std::move(fb),
        .colour_attachment_count = colour_attachment_count,
        .first_colour_attachment_index = first_index,
        .depth_attachment_index = depth_texture_index,
    };
}

auto sprite() -> ufps::MeshData
{
    const ufps::Vector3 positions[] = {
        {-1.0f, 1.0f, 0.0f}, {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};

    const ufps::UV uvs[] = {{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};

    auto indices = std::vector<std::uint32_t>{0, 1, 2, 0, 2, 3};

    return {.vertices = vertices(positions, positions, positions, positions, uvs), .indices = std::move(indices)};
}
}

namespace ufps
{

Renderer::Renderer(
    std::uint32_t width,
    std::uint32_t height,
    ResourceLoader &resource_loader,
    TextureManager &texture_manager,
    MeshManager &mesh_manager)
    : dummy_vao_{0u, [](auto e) { ::glDeleteVertexArrays(1u, &e); }}
    , command_buffer_{"gbuffer_command_buffer"}
    , post_processing_command_buffer_{"post_processing_command_buffer"}
    , post_process_sprite_{.name = "post_process_sprite", .mesh_view = mesh_manager.load(sprite()), .transform = {}, .material_index = 0u}
    , camera_buffer_{sizeof(CameraData), "camera_buffer"}
    , light_buffer_{sizeof(LightData), "light_buffer"}
    , object_data_buffer_{sizeof(ObjectData), "object_data_buffer"}
    , gbuffer_program_{create_program(
          resource_loader,
          "shaders\\gbuffer.vert",
          "gbuffer_vertex_shader",
          "shaders\\gbuffer.frag",
          "gbuffer_fragment_shader",
          "gbuffer_program")}
    , light_pass_program_{create_program(
          resource_loader,
          "shaders\\light_pass.vert",
          "light_pass_vertex_shader",
          "shaders\\light_pass.frag",
          "light_pass_fragment_shader",
          "light_pass_program")}
    , fb_sampler_{FilterType::LINEAR, FilterType::LINEAR, "fb_sampler"}
    , gbuffer_rt_{create_render_target(4u, width, height, fb_sampler_, texture_manager, "gbuffer")}
    , light_pass_rt_{create_render_target(1u, width, height, fb_sampler_, texture_manager, "light_pass")}
{
    post_processing_command_buffer_.build(post_process_sprite_);

    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);
}

auto Renderer::render(const Scene &scene) -> void
{
    gbuffer_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gbuffer_program_.use();

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
                                 [](const auto &e)
                                 {
                                     return ObjectData{
                                         .model = e.transform,
                                         .material_id_index = e.material_index,
                                         .padding = {},
                                     };
                                 }) |
                             std::ranges::to<std::vector>();
    resize_gpu_buffer(object_data, object_data_buffer_);
    object_data_buffer_.write(std::as_bytes(std::span{object_data.data(), object_data.size()}), 0zu);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, object_data_buffer_.native_handle());

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.material_manager.native_handle());

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene.texture_manager.native_handle());

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(command_buffer_.offset_bytes()),
        command_count,
        0);

    light_buffer_.write(std::as_bytes(std::span<const LightData, 1zu>{&scene.lights, 1zu}), 0zu);

    light_pass_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    light_pass_program_.use();
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 0u, gbuffer_rt_.first_colour_attachment_index + 0u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 1u, gbuffer_rt_.first_colour_attachment_index + 1u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 2u, gbuffer_rt_.first_colour_attachment_index + 2u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 3u, gbuffer_rt_.first_colour_attachment_index + 3u);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager.native_handle());
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, light_buffer_.native_handle());
    ::glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        3,
        camera_buffer_.native_handle(),
        camera_buffer_.frame_offset_bytes(),
        sizeof(CameraData));
    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, post_processing_command_buffer_.native_handle());
    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(post_processing_command_buffer_.offset_bytes()),
        1u,
        0);

    command_buffer_.advance();
    camera_buffer_.advance();
    light_buffer_.advance();
    object_data_buffer_.advance();

    light_pass_rt_.fb.unbind();

    ::glBlitNamedFramebuffer(
        light_pass_rt_.fb.native_handle(),
        0u,
        0u,
        0u,
        light_pass_rt_.fb.width(),
        light_pass_rt_.fb.height(),
        0u,
        0u,
        light_pass_rt_.fb.width(),
        light_pass_rt_.fb.height(),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
}
}
