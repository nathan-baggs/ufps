#include "graphics/renderer.h"

#include <GL/gl.h>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/scene.h"
#include "graphics/command_buffer.h"
#include "graphics/frame_buffer.h"
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
auto create_program(ufps::ResourceLoader &resource_loader) -> ufps::Program
{
    const auto sample_vert = ufps::Shader{
        resource_loader.load_string("shaders\\simple.vert"), ufps::ShaderType::VERTEX, "sample_vertex_shader"sv};
    const auto sample_frag = ufps::Shader{
        resource_loader.load_string("shaders\\simple.frag"), ufps::ShaderType::FRAGMENT, "sample_fragment_shader"sv};
    return ufps::Program{sample_vert, sample_frag, "sample_prog"sv};
}

auto create_frame_buffer(
    std::uint32_t width,
    std::uint32_t height,
    ufps::Sampler &sampler,
    ufps::TextureManager &texture_manager) -> ufps::FrameBuffer
{
    const auto fb_texture_data = ufps::TextureData{
        .width = width,
        .height = height,
        .format = ufps::TextureFormat::RGB16F,
        .data = std::nullopt,
    };
    auto fb_texture = ufps::Texture{fb_texture_data, "fb_texture", sampler};
    const auto fb_texture_index = texture_manager.add(std::move(fb_texture));

    const auto depth_texture_data = ufps::TextureData{
        .width = width,
        .height = height,
        .format = ufps::TextureFormat::DEPTH24,
        .data = std::nullopt,
    };
    auto depth_texture = ufps::Texture{depth_texture_data, "depth_texture", sampler};
    const auto depth_texture_index = texture_manager.add(std::move(depth_texture));

    return {
        texture_manager.textures({fb_texture_index}),
        texture_manager.texture(depth_texture_index),
        "main_frame_buffer"};
}

}

namespace ufps
{

Renderer::Renderer(
    std::uint32_t width,
    std::uint32_t height,
    ResourceLoader &resource_loader,
    TextureManager &texture_manager)
    : dummy_vao_{0u, [](auto e) { ::glDeleteVertexArrays(1u, &e); }}
    , command_buffer_{}
    , camera_buffer_{sizeof(CameraData), "camera_buffer"}
    , light_buffer_{sizeof(LightData), "light_buffer"}
    , object_data_buffer_{sizeof(ObjectData), "object_data_buffer"}
    , program_{create_program(resource_loader)}
    , fb_sampler_{FilterType::LINEAR, FilterType::LINEAR, "fb_sampler"}
    , fb_{create_frame_buffer(width, height, fb_sampler_, texture_manager)}
{
    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);

    program_.use();
}

auto Renderer::render(const Scene &scene) -> void
{
    fb_.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, scene.texture_manager.native_handle());

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

    fb_.unbind();

    ::glBlitNamedFramebuffer(
        fb_.native_handle(),
        0u,
        0u,
        0u,
        fb_.width(),
        fb_.height(),
        0u,
        0u,
        fb_.width(),
        fb_.height(),
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
}
}
