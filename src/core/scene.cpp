#include "core/scene.h"

#include <ranges>
#include <span>
#include <string_view>

#include "core/camera.h"
#include "core/entity.h"
#include "utils/error.h"
#include "utils/string_map.h"

namespace ufps
{
Scene::Scene(
    MeshManager &mesh_manager,
    MaterialManager &material_manager,
    TextureManager &texture_manager,
    Camera camera,
    LightData lights,
    ToneMapOptions tone_map_options,
    SSAOOptions ssao_options,
    ExposureOptions exposure_options,
    const StringMap<Entity> &entity_cache)
    : entities_{}
    , entity_cache_{}
    , mesh_manager_{mesh_manager}
    , material_manager_{material_manager}
    , texture_manager_{texture_manager}
    , camera_{std::move(camera)}
    , lights_{std::move(lights)}
    , tone_map_options_{std::move(tone_map_options)}
    , ssao_options_{std::move(ssao_options)}
    , exposure_options_{std::move(exposure_options)}
{
    for (const auto &[name, entity] : entity_cache)
    {
        cache_entity(name, entity);
    }
}

Scene::Scene(
    MeshManager &mesh_manager,
    MaterialManager &material_manager,
    TextureManager &texture_manager,
    Camera camera,
    const Scene::Description &description,
    const StringMap<Entity> &entity_cache)
    : entities_{}
    , entity_cache_{}
    , mesh_manager_{mesh_manager}
    , material_manager_{material_manager}
    , texture_manager_{texture_manager}
    , camera_{std::move(camera)}
    , lights_{description.lights}
    , tone_map_options_{description.tone_map_options}
    , ssao_options_{description.ssao_options}
    , exposure_options_{description.exposure_options}
{
    for (const auto &[name, entity] : entity_cache)
    {
        cache_entity(name, entity);
    }

    for (const auto &entity_description : description.entities)
    {
        const auto cached = std::ranges::find_if(
            entity_cache_, [&entity_description](const auto &e) { return e.name() == entity_description.name; });
        expect(cached != std::ranges::cend(entity_cache_), "unknown entity: {}", entity_description.name);

        auto &new_entity = entities_.emplace_back(*cached);
        new_entity.set_transform(entity_description.transform);
    }
}

auto Scene::create_entity(std::string_view name) -> Entity *
{
    const auto cached = std::ranges::find_if(entity_cache_, [name](const auto &e) { return e.name() == name; });
    expect(cached != std::ranges::cend(entity_cache_), "unknown entity: {}", name);

    auto &new_entity = entities_.emplace_back(*cached);
    new_entity.set_transform({});

    return &new_entity;
}

auto Scene::cache_entity(std::string_view name, Entity entity) -> void
{
    const auto cached = std::ranges::find_if(entity_cache_, [name](const auto &e) { return e.name() == name; });
    expect(cached == std::ranges::cend(entity_cache_), "{} already exists", name);

    entity_cache_.push_back(std::move(entity));
}

}
