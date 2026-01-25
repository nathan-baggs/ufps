#pragma once

#include <cstdint>
#include <ranges>
#include <span>

#include "graphics/buffer.h"
#include "graphics/utils.h"
#include "utils/error.h"

namespace ufps
{

struct MaterialData
{
    std::uint32_t albedo_texture_index;
    std::uint32_t normal_texture_index;
    std::uint32_t specular_texture_index;
};

class MaterialManager
{
  public:
    MaterialManager()
        : material_data_cpu_{}
        , material_data_gpu_(sizeof(MaterialData), "material_manager_buffer")
    {
    }

    template <class... Args>
    auto add(Args &&...args) -> std::uint32_t
    {
        const auto new_index = material_data_cpu_.size();

        material_data_cpu_.emplace_back(std::forward<Args>(args)...);

        resize_gpu_buffer(material_data_cpu_, material_data_gpu_);

        material_data_gpu_.write(std::as_bytes(std::span{material_data_cpu_.data(), material_data_cpu_.size()}), 0zu);

        return new_index;
    }

    auto data() const -> std::span<const MaterialData>
    {
        return material_data_cpu_;
    }

    auto native_handle() const
    {
        return material_data_gpu_.native_handle();
    }

  private:
    std::vector<MaterialData> material_data_cpu_;
    Buffer material_data_gpu_;
};

}
