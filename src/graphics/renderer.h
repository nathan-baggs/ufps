#pragma once

#include <cstdint>

#include "core/scene.h"
#include "graphics/command_buffer.h"
#include "graphics/frame_buffer.h"
#include "graphics/mesh_manager.h"
#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/program.h"
#include "graphics/sampler.h"
#include "graphics/texture_manager.h"
#include "graphics/window.h"
#include "resources/resource_loader.h"
#include "utils/auto_release.h"

namespace ufps
{

struct RenderTarget
{
    FrameBuffer fb;
    std::uint32_t colour_attachment_count;
    std::uint32_t first_colour_attachment_index;
    std::uint32_t depth_attachment_index;
};

class Renderer
{
  public:
    Renderer(
        const Window &window,
        ResourceLoader &resource_loader,
        TextureManager &texture_manager,
        MeshManager &mesh_manager);
    virtual ~Renderer() = default;

    auto render(Scene &scene) -> void;

  protected:
    static auto create_program(
        ufps::ResourceLoader &resource_loader,
        std::string_view vertex_path,
        std::string_view vertex_name,
        std::string_view fragment_path,
        std::string_view fragment_name,
        std::string_view program_name) -> ufps::Program;

    static auto create_program(
        ufps::ResourceLoader &resource_loader,
        std::string_view compute_path,
        std::string_view compute_name,
        std::string_view program_name) -> ufps::Program;

    virtual auto post_render(Scene &scene) -> void;

    const Window &window_;
    AutoRelease<::GLuint> dummy_vao_;
    CommandBuffer command_buffer_;
    CommandBuffer post_processing_command_buffer_;
    Entity post_process_sprite_;
    MultiBuffer<PersistentBuffer> camera_buffer_;
    MultiBuffer<PersistentBuffer> light_buffer_;
    MultiBuffer<PersistentBuffer> object_data_buffer_;
    Buffer luminance_histogram_buffer_;
    Buffer average_luminance_buffer_;
    Buffer ssao_samples_buffer_;
    Program gbuffer_program_;
    Program light_pass_program_;
    Program tone_map_program_;
    Program luminance_histogram_program_;
    Program average_luminance_program_;
    Program ssao_program_;
    Program ssao_blur_program_;
    Sampler ssao_noise_sampler_;
    std::uint32_t ssao_noise_texture_;
    Sampler fb_sampler_;
    RenderTarget gbuffer_rt_;
    RenderTarget light_pass_rt_;
    RenderTarget tone_map_rt_;
    RenderTarget ssao_rt_;
    RenderTarget ssao_blur_rt_;
    MeshManager &mesh_manager_;
    FrameBuffer *final_fb_;

  private:
    auto execute_gbuffer_pass(Scene &scene) -> void;
    auto execute_lighting_pass(Scene &scene) -> void;
    auto execute_luminance_histogram_pass(Scene &scene) -> void;
    auto execute_average_luminance_pass(Scene &scene) -> void;
    auto execute_ssao_pass(Scene &scene) -> void;
    auto execute_tone_mapping_pass(Scene &scene) -> void;
};
}
