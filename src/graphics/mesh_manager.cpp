#include "graphics/mesh_manager.h"

#include <ranges>
#include <vector>

#include "graphics/vertex_data.h"
#include "utils/data_buffer.h"
#include "utils/log.h"

namespace ufps
{
MeshManager::MeshManager()
    : mesh_data_cpu_{}
    , mesh_data_gpu_{sizeof(VertexData), "mesh_data"}
{
    mesh_data_cpu_.reserve(1u);
}

auto MeshManager::load(const std::vector<VertexData> &mesh) -> MeshView
{
    const auto offset = mesh_data_cpu_.size();

    mesh_data_cpu_.append_range(mesh);

    const auto buffer_size_bytes = mesh_data_cpu_.size() * sizeof(VertexData);

    if (mesh_data_gpu_.size() <= buffer_size_bytes)
    {
        auto new_size = mesh_data_gpu_.size() * 2zu;
        while (new_size < buffer_size_bytes)
        {
            new_size *= 2zu;
        }

        log::info("growing mesh_data buffer {:x} -> {:x}", mesh_data_gpu_.size(), new_size);
        mesh_data_gpu_ = Buffer{new_size, "mesh_data"};
    }

    auto mesh_view = DataBufferView{reinterpret_cast<const std::byte *>(mesh_data_cpu_.data()), buffer_size_bytes};
    mesh_data_gpu_.write(mesh_view, 0u);

    return {.offset = static_cast<std::uint32_t>(offset), .count = static_cast<std::uint32_t>(mesh.size())};
}

auto MeshManager::native_handle() const -> ::GLuint
{
    return mesh_data_gpu_.native_handle();
}

auto MeshManager::to_string() const -> std::string
{
    return std::format("mesh manager: vertex count: {:x}", mesh_data_cpu_.size());
}

}
