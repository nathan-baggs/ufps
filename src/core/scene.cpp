#include "core/scene.h"

#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/entity.h"
#include "utils/error.h"

namespace ufps
{
Scene::Scene(
    MeshManager &mesh_manager,
    MaterialManager &material_manager,
    TextureManager &texture_manager,
    Camera camera,
    LightData lights)
    : entities_{}
    , entity_cache_{}
    , mesh_manager_{mesh_manager}
    , material_manager_{material_manager}
    , texture_manager_{texture_manager}
    , camera_{std::move(camera)}
    , lights_{std::move(lights)}
{
}

auto Scene::create_entity(std::string_view name) -> void
{
    const auto cached = std::ranges::find_if(entity_cache_, [name](const auto &e) { return e.name() == name; });
    expect(cached != std::ranges::cend(entity_cache_), "unknown entity: {}", name);

    auto &new_entity = entities_.emplace_back(*cached);
    new_entity.set_transform({});
}

auto Scene::cache_entity(std::string_view name, Entity entity) -> void
{
    const auto cached = std::ranges::find_if(entity_cache_, [name](const auto &e) { return e.name() == name; });
    expect(cached == std::ranges::cend(entity_cache_), "{} already exists", name);

    entity_cache_.push_back(std::move(entity));
}

auto Scene::camera() -> Camera &
{
    return camera_;
}

auto Scene::lights() -> LightData &
{
    return lights_;
}

auto Scene::mesh_manager() const -> MeshManager &
{
    return mesh_manager_;
}

auto Scene::material_manager() const -> MaterialManager &
{
    return material_manager_;
}

auto Scene::texture_manager() const -> TextureManager &
{
    return texture_manager_;
}

}
