#pragma once

#include <GL/gl.h>
#include <cstdint>
#include <string>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/vertex_data.h"

namespace ufps
{

struct MeshView
{
    std::uint32_t offset;
    std::uint32_t count;
};

class MeshManager
{
  public:
    MeshManager();
    auto load(const std::vector<VertexData> &mesh) -> MeshView;

    auto native_handle() const -> ::GLuint;

    auto to_string() const -> std::string;

  private:
    std::vector<VertexData> mesh_data_cpu_;
    Buffer mesh_data_gpu_;
};

}
