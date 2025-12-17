#pragma once

#include <compare>
#include <cstdint>
#include <flat_map>
#include <ranges>
#include <string>

#include "graphics/colour.h"
#include "graphics/multi_buffer.h"
#include "graphics/persistent_buffer.h"
#include "graphics/utils.h"
#include "utils/error.h"
#include "utils/formatter.h"

namespace ufps
{

class MaterialKey
{
  public:
    constexpr MaterialKey(std::uint32_t key)
        : key_{key}
    {
    }

    constexpr auto get() const -> std::uint32_t
    {
        return key_;
    }

    constexpr auto operator*() const -> std::uint32_t
    {
        return get();
    }

    constexpr auto operator<=>(const MaterialKey &) const = default;

    constexpr auto to_string() const -> std::string
    {
        return std::to_string(key_);
    }

  private:
    std::uint32_t key_;
};

struct MaterialData
{
    Colour colour;
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
    auto add(Args &&...args) -> MaterialKey
    {
        static auto key_num = 0u;
        const auto key = MaterialKey{key_num++};

        material_data_cpu_.emplace(key, std::forward<Args>(args)...);
        resize_gpu_buffer(data(), material_data_gpu_, "material_manager_buffer");

        return key;
    }

    auto operator[](MaterialKey key) -> MaterialData &
    {
        const auto element = material_data_cpu_.find(key);
        expect(element != std::ranges::cend(material_data_cpu_), "key {} does not exist", key);

        return element->second;
    }

    auto remove(MaterialKey key) -> void
    {
        material_data_cpu_.erase(key);
    }

    auto index(MaterialKey key) -> std::uint32_t
    {
        const auto element = material_data_cpu_.find(key);
        expect(element != std::ranges::cend(material_data_cpu_), "could not find key: {}", key);

        return static_cast<std::uint32_t>(std::ranges::distance(std::ranges::cbegin(material_data_cpu_), element));
    }

    auto data() const -> const std::vector<MaterialData> &
    {
        return material_data_cpu_.values();
    }

    auto sync() -> void
    {
        const auto &values = material_data_cpu_.values();
        material_data_gpu_.write(std::as_bytes(std::span{values.data(), values.size()}), 0zu);
    }

    auto native_handle() const
    {
        return material_data_gpu_.native_handle();
    }

    auto advance() -> void
    {
        material_data_gpu_.advance();
    }

  private:
    std::flat_map<MaterialKey, MaterialData> material_data_cpu_;
    MultiBuffer<PersistentBuffer> material_data_gpu_;
};

}
