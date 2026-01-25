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
    virtual auto post_render(Scene &scene) -> void;

    const Window &window_;
    AutoRelease<::GLuint> dummy_vao_;
    CommandBuffer command_buffer_;
    CommandBuffer post_processing_command_buffer_;
    Entity post_process_sprite_;
    MultiBuffer<PersistentBuffer> camera_buffer_;
    MultiBuffer<PersistentBuffer> light_buffer_;
    MultiBuffer<PersistentBuffer> object_data_buffer_;
    Program gbuffer_program_;
    Program light_pass_program_;
    Sampler fb_sampler_;
    RenderTarget gbuffer_rt_;
    RenderTarget light_pass_rt_;
};
}
