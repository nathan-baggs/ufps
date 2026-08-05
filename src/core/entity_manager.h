#pragma once

#include <ranges>
#include <vector>

#include "core/entity.h"
#include "core/sparse_set.h"
#include "utils/error.h"
#include "utils/string_map.h"

namespace ufps
{

using EntityHandle = SparseSet<Entity>::handle_type;

class EntityManager
{
  public:
    constexpr auto register_entity(std::string_view name, Entity entity) -> EntityHandle;

    constexpr auto operator[](std::string_view name) -> EntityHandle;

    constexpr auto operator[](EntityHandle handle);

  private:
    SparseSet<Entity> entities_;
    StringMap<EntityHandle> entity_names_;
};

constexpr auto EntityManager::register_entity(std::string_view name, Entity entity) -> EntityHandle
{
    auto handle = entities_.emplace(std::move(entity));
    entity_names_[std::string{name}] = handle;
    return handle;
}

constexpr auto EntityManager::operator[](std::string_view name) -> EntityHandle
{
    auto element = entity_names_.find(name);
    ensure(element != std::ranges::cend(entity_names_), "{} does not exist", name);

    return element->second;
}

constexpr auto EntityManager::operator[](EntityHandle handle)
{
    return entities_[handle];
}

}
