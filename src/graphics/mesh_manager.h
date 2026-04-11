#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "graphics/buffer.h"
#include "graphics/mesh_data.h"
#include "graphics/mesh_view.h"
#include "graphics/vertex_data.h"
#include "utils/data_buffer.h"
#include "utils/string_map.h"

namespace ufps
{

class MeshManager
{
  public:
    MeshManager();

    MeshManager(
        std::vector<VertexData> vertex_data,
        std::vector<std::uint32_t> index_data,
        StringMap<std::vector<MeshView>> mesh_lookup);

    MeshManager(
        DataBufferView raw_vertex_data,
        DataBufferView raw_index_data,
        StringMap<std::vector<MeshView>> mesh_lookup);

    auto load(std::string_view name, std::span<const MeshData> mesh_data) -> std::span<const MeshView>;

    auto mesh(std::string_view name) -> std::span<const MeshView>;

    auto mesh_names() const -> std::vector<std::string>;

    auto native_handle() const -> std::tuple<::GLuint, ::GLuint>;

    auto index_data(MeshView view) const -> std::span<const std::uint32_t>;

    auto vertex_data(MeshView view) const -> std::span<const VertexData>;

    auto to_string() const -> std::string;

  private:
    std::vector<VertexData> vertex_data_cpu_;
    std::vector<std::uint32_t> index_data_cpu_;
    Buffer vertex_data_gpu_;
    Buffer index_data_gpu_;
    StringMap<std::vector<MeshView>> mesh_lookup_;
};

}
