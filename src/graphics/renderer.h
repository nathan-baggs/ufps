#pragma once

#include "graphics/command_buffer.h"
#include "graphics/multi_buffer.h"
#include "graphics/opengl.h"
#include "graphics/persistent_buffer.h"
#include "graphics/program.h"
#include "graphics/scene.h"
#include "utils/auto_release.h"

namespace ufps
{
class Renderer
{
  public:
    Renderer();

    auto render(const Scene &scene) -> void;

  private:
    AutoRelease<::GLuint> dummy_vao_;
    CommandBuffer command_buffer_;
    MultiBuffer<PersistentBuffer> camera_buffer_;
    MultiBuffer<PersistentBuffer> object_data_buffer_;
    Program program_;
};
}
