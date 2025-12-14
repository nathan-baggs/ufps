#include "graphics/mesh_manager.h"

#include <ranges>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/mesh_data.h"
#include "graphics/utils.h"
#include "graphics/vertex_data.h"
#include "utils/data_buffer.h"

namespace ufps
{
MeshManager::MeshManager()
    : vertex_data_cpu_{}
    , index_data_cpu_{}
    , vertex_data_gpu_{sizeof(VertexData), "vertex_mesh_data"}
    , index_data_gpu_{sizeof(std::uint32_t), "index_mesh_data"}
{
}

auto MeshManager::load(const MeshData &mesh_data) -> MeshView
{
    const auto vertex_offset = vertex_data_cpu_.size();
    const auto index_offset = index_data_cpu_.size();

    vertex_data_cpu_.append_range(mesh_data.vertices);
    resize_gpu_buffer(vertex_data_cpu_, vertex_data_gpu_, "vertex_mesh_data");
    const auto vertex_data_view = DataBufferView{
        reinterpret_cast<const std::byte *>(vertex_data_cpu_.data()), vertex_data_cpu_.size() * sizeof(VertexData)};
    vertex_data_gpu_.write(vertex_data_view, 0u);

    index_data_cpu_.append_range(mesh_data.indices);
    resize_gpu_buffer(index_data_cpu_, index_data_gpu_, "index_mesh_data");
    const auto index_data_view = DataBufferView{
        reinterpret_cast<const std::byte *>(index_data_cpu_.data()), index_data_cpu_.size() * sizeof(std::uint32_t)};
    index_data_gpu_.write(index_data_view, 0u);

    return {
        .index_offset = static_cast<std::uint32_t>(index_offset),
        .index_count = static_cast<std::uint32_t>(mesh_data.indices.size()),
        .vertex_offset = static_cast<std::uint32_t>(vertex_offset),
        .vertex_count = static_cast<std::uint32_t>(mesh_data.vertices.size()),
    };
}

auto MeshManager::native_handle() const -> std::tuple<::GLuint, ::GLuint>
{
    return {vertex_data_gpu_.native_handle(), index_data_gpu_.native_handle()};
}

auto MeshManager::to_string() const -> std::string
{
    return std::format(
        "mesh manager: vertex count: {} index count: {}", vertex_data_cpu_.size(), index_data_cpu_.size());
}

}
