#pragma once

#include <GL/gl.h>
#include <cstdint>
#include <string>

#include "graphics/buffer.h"
#include "graphics/mesh_data.h"
#include "graphics/mesh_view.h"
#include "graphics/vertex_data.h"

namespace ufps
{

class MeshManager
{
  public:
    MeshManager();
    auto load(const MeshData &mesh_data) -> MeshView;

    auto native_handle() const -> std::tuple<::GLuint, ::GLuint>;

    auto to_string() const -> std::string;

  private:
    std::vector<VertexData> vertex_data_cpu_;
    std::vector<std::uint32_t> index_data_cpu_;
    Buffer vertex_data_gpu_;
    Buffer index_data_gpu_;
};

}
