#include "graphics/mesh_manager.h"

#include <string_view>
#include <vector>

#include "graphics/buffer.h"
#include "graphics/mesh_data.h"
#include "graphics/utils.h"
#include "graphics/vertex_data.h"
#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/string_map.h"

namespace ufps
{
MeshManager::MeshManager()
    : vertex_data_cpu_{}
    , index_data_cpu_{}
    , vertex_data_gpu_{sizeof(VertexData), "vertex_mesh_data"}
    , index_data_gpu_{sizeof(std::uint32_t), "index_mesh_data"}
    , mesh_lookup_{}
{
}

MeshManager::MeshManager(
    std::vector<VertexData> vertex_data,
    std::vector<std::uint32_t> index_data,
    StringMap<std::vector<MeshView>> mesh_lookup)
    : vertex_data_cpu_{std::move(vertex_data)}
    , index_data_cpu_{std::move(index_data)}
    , vertex_data_gpu_{sizeof(VertexData), "vertex_mesh_data"}
    , index_data_gpu_{sizeof(std::uint32_t), "index_mesh_data"}
    , mesh_lookup_{std::move(mesh_lookup)}
{
    resize_gpu_buffer(vertex_data_cpu_, vertex_data_gpu_);
    const auto vertex_data_view = DataBufferView{
        reinterpret_cast<const std::byte *>(vertex_data_cpu_.data()), vertex_data_cpu_.size() * sizeof(VertexData)};
    vertex_data_gpu_.write(vertex_data_view, 0u);

    resize_gpu_buffer(index_data_cpu_, index_data_gpu_);
    const auto index_data_view = DataBufferView{
        reinterpret_cast<const std::byte *>(index_data_cpu_.data()), index_data_cpu_.size() * sizeof(std::uint32_t)};
    index_data_gpu_.write(index_data_view, 0u);
}

MeshManager::MeshManager(
        DataBufferView raw_vertex_data,
        DataBufferView raw_index_data,
        StringMap<std::vector<MeshView>> mesh_lookup)
    : vertex_data_cpu_{reinterpret_cast<const VertexData *>(raw_vertex_data.data()),
                       reinterpret_cast<const VertexData *>(raw_vertex_data.data() + raw_vertex_data.size())}
    , index_data_cpu_{reinterpret_cast<const std::uint32_t *>(raw_index_data.data()),
                      reinterpret_cast<const std::uint32_t *>(raw_index_data.data() + raw_index_data.size())}
    , vertex_data_gpu_{sizeof(VertexData), "vertex_mesh_data"}
    , index_data_gpu_{sizeof(std::uint32_t), "index_mesh_data"}
    , mesh_lookup_{std::move(mesh_lookup)}
{
    resize_gpu_buffer(vertex_data_cpu_, vertex_data_gpu_);
    vertex_data_gpu_.write(raw_vertex_data, 0u);

    resize_gpu_buffer(index_data_cpu_, index_data_gpu_);
    index_data_gpu_.write(raw_index_data, 0u);
}

auto MeshManager::load(std::string_view name, std::span<const MeshData> meshes) -> std::span<const MeshView>
{
    expect(!mesh_lookup_.contains(name), "{} mesh exists", name);

    auto mesh_views = std::vector<MeshView>{};

    for (const auto &mesh_data : meshes)
    {
        const auto vertex_offset = vertex_data_cpu_.size();
        const auto index_offset = index_data_cpu_.size();

        vertex_data_cpu_.append_range(mesh_data.vertices);
        resize_gpu_buffer(vertex_data_cpu_, vertex_data_gpu_);
        const auto vertex_data_view = DataBufferView{
            reinterpret_cast<const std::byte *>(vertex_data_cpu_.data()), vertex_data_cpu_.size() * sizeof(VertexData)};
        vertex_data_gpu_.write(vertex_data_view, 0u);

        index_data_cpu_.append_range(mesh_data.indices);
        resize_gpu_buffer(index_data_cpu_, index_data_gpu_);
        const auto index_data_view = DataBufferView{
            reinterpret_cast<const std::byte *>(index_data_cpu_.data()),
            index_data_cpu_.size() * sizeof(std::uint32_t)};
        index_data_gpu_.write(index_data_view, 0u);

        mesh_views.push_back({
            .index_offset = static_cast<std::uint32_t>(index_offset),
            .index_count = static_cast<std::uint32_t>(mesh_data.indices.size()),
            .vertex_offset = static_cast<std::uint32_t>(vertex_offset),
            .vertex_count = static_cast<std::uint32_t>(mesh_data.vertices.size()),
        });
    }

    const auto &[iter, _] = mesh_lookup_.emplace(name, mesh_views);

    return iter->second;
}

auto MeshManager::mesh(std::string_view name) -> std::span<const MeshView>
{
    auto mesh_view = mesh_lookup_.find(name);
    expect(mesh_view != std::ranges::cend(mesh_lookup_), "{} mesh does not exist", name);

    return mesh_view->second;
}

auto MeshManager::mesh_names() const -> std::vector<std::string>
{
    return mesh_lookup_ | std::views::keys | std::ranges::to<std::vector>();
}

auto MeshManager::native_handle() const -> std::tuple<::GLuint, ::GLuint>
{
    return {vertex_data_gpu_.native_handle(), index_data_gpu_.native_handle()};
}

auto MeshManager::index_data(MeshView view) const -> std::span<const std::uint32_t>
{
    return {index_data_cpu_.data() + view.index_offset, view.index_count};
}

auto MeshManager::vertex_data(MeshView view) const -> std::span<const VertexData>
{
    return {vertex_data_cpu_.data() + view.vertex_offset, view.vertex_count};
}

auto MeshManager::to_string() const -> std::string
{
    return std::format(
        "mesh manager: vertex count: {} index count: {}", vertex_data_cpu_.size(), index_data_cpu_.size());
}

}
