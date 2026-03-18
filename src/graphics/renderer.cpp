#include "graphics/renderer.h"

#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/entity.h"
#include "core/scene.h"
#include "graphics/buffer_writer.h"
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

auto create_sprite(ufps::MeshManager &mesh_manager) -> ufps::Entity
{
    const auto mesh_data = std::vector{sprite()};
    const auto mesh_views = mesh_manager.load("sprite", mesh_data);
    return {"post_process_sprite", {{mesh_views.front(), 0u, mesh_manager}}, {}};
}

}

namespace ufps
{

Renderer::Renderer(
    const Window &window,
    ResourceLoader &resource_loader,
    TextureManager &texture_manager,
    MeshManager &mesh_manager)
    : window_{window}
    , dummy_vao_{0u, [](auto e) { ::glDeleteVertexArrays(1u, &e); }}
    , command_buffer_{"gbuffer_command_buffer"}
    , post_processing_command_buffer_{"post_processing_command_buffer"}
    , post_process_sprite_{create_sprite(mesh_manager)}
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
    , tone_map_program_{create_program(
          resource_loader,
          "shaders\\tone_map.vert",
          "tone_map_vertex_shader",
          "shaders\\tone_map.frag",
          "tone_map_fragment_shader",
          "tone_map_program")}
    , fb_sampler_{FilterType::LINEAR, FilterType::LINEAR, "fb_sampler"}
    , gbuffer_rt_{create_render_target(
          4u,
          window_.render_width(),
          window_.render_height(),
          fb_sampler_,
          texture_manager,
          "gbuffer")}
    , light_pass_rt_{create_render_target(
          1u,
          window_.render_width(),
          window_.render_height(),
          fb_sampler_,
          texture_manager,
          "light_pass"),}
    , tone_map_rt_{create_render_target(
          1u,
          window_.render_width(),
          window_.render_height(),
          fb_sampler_,
          texture_manager,
          "tone_map"),},
    mesh_manager_{mesh_manager},
    final_fb_{}
{
    post_processing_command_buffer_.build(post_process_sprite_);

    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);

    // ac15CR: this code is not reasonable... but our discord is: https://discord.gg/9FkkMgXSUV
}

auto Renderer::render(Scene &scene) -> void
{
    gbuffer_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gbuffer_program_.use();

    camera_buffer_.write(scene.camera().data_view(), 0zu);

    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager().native_handle();
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

    auto object_data = std::vector<ObjectData>{};

    for (const auto &entity : scene.entities())
    {
        object_data.append_range(
            entity.render_entities() | std::views::transform(
                                           [&entity](const auto &e)
                                           {
                                               return ObjectData{
                                                   .model = entity.transform(),
                                                   .material_id_index = e.material_index(),
                                                   .padding = {},
                                               };
                                           }));
    }

    resize_gpu_buffer(object_data, object_data_buffer_);
    object_data_buffer_.write(std::as_bytes(std::span{object_data.data(), object_data.size()}), 0zu);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, object_data_buffer_.native_handle());

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, scene.material_manager().native_handle());

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, scene.texture_manager().native_handle());

    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(command_buffer_.offset_bytes()),
        command_count,
        0);

    light_buffer_.write(std::as_bytes(std::span<const LightData, 1zu>{&scene.lights(), 1zu}), 0zu);

    light_pass_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    light_pass_program_.use();

    {
        const auto &lights = scene.lights();

        const auto buffer_size_bytes =
            sizeof(lights.ambient) + sizeof(std::uint32_t) + sizeof(PointLight) * lights.lights.size();
        if (light_buffer_.size() < buffer_size_bytes)
        {
            light_buffer_ = {buffer_size_bytes, light_buffer_.name()};
            // opengl barrier incase gpu using previous frame
            ::glFinish();
        }

        auto writer = BufferWriter{light_buffer_};
        writer.write(lights.ambient);
        writer.write(static_cast<std::uint32_t>(lights.lights.size()));
        writer.write<PointLight>(lights.lights);
    }

    ::glProgramUniform1ui(light_pass_program_.native_handle(), 0u, gbuffer_rt_.first_colour_attachment_index + 0u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 1u, gbuffer_rt_.first_colour_attachment_index + 1u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 2u, gbuffer_rt_.first_colour_attachment_index + 2u);
    ::glProgramUniform1ui(light_pass_program_.native_handle(), 3u, gbuffer_rt_.first_colour_attachment_index + 3u);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager().native_handle());
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

    tone_map_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    tone_map_program_.use();

    ::glProgramUniform1ui(tone_map_program_.native_handle(), 0u, light_pass_rt_.first_colour_attachment_index);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager().native_handle());
    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, post_processing_command_buffer_.native_handle());
    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(post_processing_command_buffer_.offset_bytes()),
        1u,
        0);

    final_fb_ = &tone_map_rt_.fb;

    post_render(scene);

    command_buffer_.advance();
    camera_buffer_.advance();
    light_buffer_.advance();
    object_data_buffer_.advance();
}

auto Renderer::post_render(Scene &) -> void
{
    final_fb_->unbind();

    ::glBlitNamedFramebuffer(
        final_fb_->native_handle(),
        0u,
        0u,
        0u,
        final_fb_->width(),
        final_fb_->height(),
        0u,
        0u,
        final_fb_->width(),
        final_fb_->height(),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
}

auto Renderer::create_program(
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
}
