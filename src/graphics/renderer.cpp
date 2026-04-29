#include "graphics/renderer.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <random>
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

template <class T>
struct AutoBind
{
    AutoBind(T &obj)
        : obj{obj}
    {
        obj.bind();
    }

    ~AutoBind()
    {
        obj.unbind();
    }

    T &obj;
};

auto create_render_target(
    std::uint32_t colour_attachment_count,
    std::uint32_t width,
    std::uint32_t height,
    ufps::Sampler &sampler,
    ufps::TextureManager &texture_manager,
    std::string_view name,
    ufps::TextureFormat format = ufps::TextureFormat::RGB16F) -> ufps::RenderTarget
{
    const auto colour_attachment_texture_data = ufps::TextureData{
        .width = width,
        .height = height,
        .format = format,
        .data = std::nullopt,
        .is_compressed = false,
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
        .is_compressed = false,
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

auto create_ssao_noise_texture(ufps::TextureManager &texture_manager, const ufps::Sampler &sampler) -> std::uint32_t
{
    auto generator = std::mt19937{std::random_device{}()};
    auto distribution = std::uniform_real_distribution<float>{-1.0f, 1.0f};

    auto ssao_noise_data = ufps::DataBuffer{};

    for (auto i = 0u; i < 16u; ++i)
    {
        const auto x = distribution(generator);
        const auto y = distribution(generator);
        const auto z = 0.0f;

        ssao_noise_data.push_back(static_cast<std::byte>((x * 0.5f + 0.5f) * 255.0f));
        ssao_noise_data.push_back(static_cast<std::byte>((y * 0.5f + 0.5f) * 255.0f));
        ssao_noise_data.push_back(static_cast<std::byte>((z * 0.5f + 0.5f) * 255.0f));
    }

    const auto ssao_noise_texture_data = ufps::TextureData{
        .width = 4,
        .height = 4,
        .format = ufps::TextureFormat::RGB,
        .data = ssao_noise_data,
        .is_compressed = false,
    };

    auto tex = ufps::Texture{
        ssao_noise_texture_data,
        "ssao_noise_texture",
        sampler,
    };

    return texture_manager.add(std::move(tex));
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
    , luminance_histogram_buffer_{sizeof(std::uint32_t) * 256, "luminance_histogram_buffer"}
    , average_luminance_buffer_{sizeof(float), "average_luminance_buffer"}
    , ssao_samples_buffer_{sizeof(Vector4) * 64, "ssao_samples_buffer"}
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
    , luminance_histogram_program_{create_program(
          resource_loader,
          "shaders\\luminance_histogram.comp",
          "luminance_histogram_shader",
          "luminance_histogram_program")}
    , average_luminance_program_{create_program(
          resource_loader,
          "shaders\\average_luminance.comp",
          "average_luminance_shader",
          "average_luminance_program")}
    , ssao_program_{create_program(
          resource_loader,
          "shaders\\ssao.vert",
          "ssao_vertex_shader",
          "shaders\\ssao.frag",
          "ssao_fragment_shader",
          "ssao_program")}
    , ssao_blur_program_{create_program(
          resource_loader,
          "shaders\\ssao.vert",
          "ssao_blur_vertex_shader",
          "shaders\\ssao_blur.frag",
          "ssao_blur_fragment_shader",
          "ssao_blur_program")}
    , ssao_noise_sampler_{FilterType::NEAREST, FilterType::NEAREST, WrapMode::REPEAT, WrapMode::REPEAT, "ssao_noise_sampler"}
    , ssao_noise_texture_{create_ssao_noise_texture(texture_manager, ssao_noise_sampler_)}
    , fb_sampler_{FilterType::LINEAR, FilterType::LINEAR, WrapMode::CLAMP_TO_EDGE, WrapMode::CLAMP_TO_EDGE, "fb_sampler"}
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
          "tone_map"),}
    , ssao_rt_{create_render_target(
          1u,
          window_.render_width() / 2u,
          window_.render_height() / 2u,
          fb_sampler_,
          texture_manager,
          "ssao",
          TextureFormat::R16F),}
    , ssao_blur_rt_{create_render_target(
          1u,
          window_.render_width() / 2u,
          window_.render_height() / 2u,
          fb_sampler_,
          texture_manager,
          "ssao_blur",
          TextureFormat::R16F),}
    ,mesh_manager_{mesh_manager}
    ,final_fb_{}
{
    post_processing_command_buffer_.build(post_process_sprite_);

    ::glGenVertexArrays(1, &dummy_vao_);
    ::glBindVertexArray(dummy_vao_);

    // ac15CR: this code is not reasonable... but our discord is: https://discord.gg/9FkkMgXSUV

    auto generator = std::mt19937{std::random_device{}()};
    auto distribution = std::uniform_real_distribution<float>{0.0f, 1.0f};

    auto ssao_samples = std::vector<Vector4>{};
    for (auto i = 0u; i < 64u; ++i)
    {
        auto sample = Vector3{
            distribution(generator) * 2.0f - 1.0f,
            distribution(generator) * 2.0f - 1.0f,
            distribution(generator),
        };
        sample = Vector3::normalise(sample);
        sample *= distribution(generator);

        auto scale = static_cast<float>(i) / 64.0f;
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;

        ssao_samples.push_back(Vector4{sample, 0.0f});
    }

    ssao_samples_buffer_.write(std::as_bytes(std::span{ssao_samples.data(), ssao_samples.size()}), 0u);
}

auto Renderer::render(Scene &scene) -> void
{
    camera_buffer_.write(scene.camera().data_view(), 0zu);

    execute_gbuffer_pass(scene);
    execute_lighting_pass(scene);
    execute_luminance_histogram_pass(scene);
    execute_average_luminance_pass(scene);
    execute_ssao_pass(scene);
    execute_tone_mapping_pass(scene);

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

auto Renderer::create_program(
    ufps::ResourceLoader &resource_loader,
    std::string_view compute_path,
    std::string_view compute_name,
    std::string_view program_name) -> ufps::Program
{
    const auto compute_shader =
        ufps::Shader{resource_loader.load_string(compute_path), ufps::ShaderType::COMPUTE, compute_name};
    return ufps::Program{compute_shader, program_name};
}

auto Renderer::execute_gbuffer_pass(Scene &scene) -> void
{
    gbuffer_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto auto_bind = AutoBind{gbuffer_program_};

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
}

auto Renderer::execute_lighting_pass(Scene &scene) -> void
{
    light_pass_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto auto_bind = AutoBind{light_pass_program_};

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
        writer.write(lights.lights.data());
    }

    light_pass_program_.set_uniforms(
        gbuffer_rt_.first_colour_attachment_index + 0u,
        gbuffer_rt_.first_colour_attachment_index + 1u,
        gbuffer_rt_.first_colour_attachment_index + 2u,
        gbuffer_rt_.first_colour_attachment_index + 3u);

    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager().native_handle();
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
}

auto Renderer::execute_luminance_histogram_pass(Scene &scene) -> void
{
    const auto auto_bind = AutoBind{luminance_histogram_program_};

    const auto zero = ::GLuint{0};
    ::glClearNamedBufferData(
        luminance_histogram_buffer_.native_handle(), GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, scene.texture_manager().native_handle());
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, luminance_histogram_buffer_.native_handle());

    luminance_histogram_program_.set_uniforms(
        light_pass_rt_.first_colour_attachment_index,
        scene.exposure_options().min_log_luminance,
        1.0f / (scene.exposure_options().max_log_luminance - scene.exposure_options().min_log_luminance));

    ::glDispatchCompute(
        static_cast<std::uint32_t>(light_pass_rt_.fb.width() + 15) / 16,
        static_cast<std::uint32_t>(light_pass_rt_.fb.height() + 15) / 16,
        1);

    ::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

auto Renderer::execute_average_luminance_pass(Scene &scene) -> void
{
    static auto delta_time = 1.0f / 60.0f;

    const auto auto_bind = AutoBind{average_luminance_program_};

    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, luminance_histogram_buffer_.native_handle());
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, average_luminance_buffer_.native_handle());

    average_luminance_program_.set_uniforms(
        scene.exposure_options().min_log_luminance,
        scene.exposure_options().max_log_luminance - scene.exposure_options().min_log_luminance,
        std::clamp(1.0f - std::exp(-delta_time * 3.0f), 0.0f, 1.0f),
        static_cast<float>(light_pass_rt_.fb.width() * light_pass_rt_.fb.height()));

    ::glDispatchCompute(1, 1, 1);

    ::glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
}

auto Renderer::execute_ssao_pass(Scene &scene) -> void
{
    if (!scene.ssao_options().enabled)
    {
        ::glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        ssao_blur_rt_.fb.bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        return;
    }

    ::glViewport(0, 0, ssao_rt_.fb.width(), ssao_rt_.fb.height());

    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager().native_handle();

    {
        ssao_rt_.fb.bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto auto_bind = AutoBind{ssao_program_};

        ssao_program_.set_uniforms(
            gbuffer_rt_.first_colour_attachment_index + 1,
            gbuffer_rt_.first_colour_attachment_index + 2,
            static_cast<float>(gbuffer_rt_.fb.width()),
            static_cast<float>(gbuffer_rt_.fb.height()),
            scene.ssao_options().sample_count,
            scene.ssao_options().radius,
            scene.ssao_options().bias,
            scene.ssao_options().power,
            ssao_noise_texture_);
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager().native_handle());
        ::glBindBufferRange(
            GL_SHADER_STORAGE_BUFFER,
            2,
            camera_buffer_.native_handle(),
            camera_buffer_.frame_offset_bytes(),
            sizeof(CameraData));
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssao_samples_buffer_.native_handle());
        ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, post_processing_command_buffer_.native_handle());
        ::glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(post_processing_command_buffer_.offset_bytes()),
            1u,
            0);
    }

    {
        ssao_blur_rt_.fb.bind();
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto auto_bind = AutoBind{ssao_blur_program_};

        ssao_blur_program_.set_uniforms(
            ssao_rt_.first_colour_attachment_index,
            gbuffer_rt_.depth_attachment_index,
            static_cast<float>(ssao_rt_.fb.width()),
            static_cast<float>(ssao_rt_.fb.height()));
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager().native_handle());
        ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, post_processing_command_buffer_.native_handle());
        ::glMultiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(post_processing_command_buffer_.offset_bytes()),
            1u,
            0);
    }

    ::glViewport(0, 0, window_.render_width(), window_.render_height());
}

auto Renderer::execute_tone_mapping_pass(Scene &scene) -> void
{
    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager().native_handle();

    tone_map_rt_.fb.bind();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto auto_bind = AutoBind{tone_map_program_};

    tone_map_program_.set_uniforms(
        light_pass_rt_.first_colour_attachment_index,
        scene.tone_map_options().max_brightness,
        scene.tone_map_options().contrast,
        scene.tone_map_options().linear_section_start,
        scene.tone_map_options().linear_section_length,
        scene.tone_map_options().black_tightness,
        scene.tone_map_options().pedestal,
        scene.tone_map_options().gamma,
        ssao_blur_rt_.first_colour_attachment_index);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scene.texture_manager().native_handle());
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, average_luminance_buffer_.native_handle());
    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, post_processing_command_buffer_.native_handle());
    ::glMultiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(post_processing_command_buffer_.offset_bytes()),
        1u,
        0);
}

}
