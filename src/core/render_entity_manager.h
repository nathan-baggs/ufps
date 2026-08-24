#pragma once

#include <ranges>
#include <vector>

#include "core/render_entity.h"
#include "core/sparse_set.h"
#include "utils/error.h"
#include "utils/string_map.h"

namespace ufps
{

using RenderEntityHandle = SparseSet<RenderEntity>::handle_type;

class RenderEntityManager
{
  public:
    constexpr auto register_group(std::string_view name, std::vector<RenderEntity> entities) -> void;

    constexpr auto operator[](std::string_view name) -> std::vector<RenderEntityHandle>;

    constexpr auto operator[](RenderEntityHandle handle);

    constexpr auto groups();

  private:
    SparseSet<RenderEntity> entities_;
    StringMap<std::vector<RenderEntityHandle>> entity_groups_;
};

constexpr auto RenderEntityManager::register_group(std::string_view name, std::vector<RenderEntity> entities) -> void
{
    auto handles = entities | std::views::transform([this](auto &e) { return entities_.emplace(std::move(e)); }) |
                   std::ranges::to<std::vector>();

    entity_groups_[std::string{name}] = std::move(handles);
}

constexpr auto RenderEntityManager::operator[](std::string_view name) -> std::vector<RenderEntityHandle>
{
    auto element = entity_groups_.find(name);
    ensure(element != std::ranges::cend(entity_groups_), "{} does not exist", name);

    return element->second;
}

constexpr auto RenderEntityManager::operator[](RenderEntityHandle handle)
{
    return entities_[handle];
}

constexpr auto RenderEntityManager::groups()
{
    return std::views::keys(entity_groups_);
}

}
