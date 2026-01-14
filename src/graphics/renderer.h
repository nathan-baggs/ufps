#pragma once

#include <cstdint>

#include "core/scene.h"
#include "graphics/command_buffer.h"
#include "graphics/frame_buffer.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_manager.h"
#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/program.h"
#include "graphics/sampler.h"
#include "graphics/texture_manager.h"
#include "resources/resource_loader.h"
#include "utils/auto_release.h"

namespace ufps
{
class Renderer
{
  public:
    Renderer(
        std::uint32_t width,
        std::uint32_t height,
        ResourceLoader &resource_loader,
        TextureManager &texture_manager,
        MeshManager &mesh_manager);

    auto render(const Scene &scene) -> void;

  private:
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
    std::uint32_t fb_texture_index_;
    FrameBuffer fb_;
    std::uint32_t light_pass_texture_index_;
    FrameBuffer light_pass_fb_;
};
}
