#pragma once

#include <algorithm>
#include <deque>
#include <ios>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

#include "core/entity.h"
#include "core/entity_manager.h"
#include "core/light_manager.h"
#include "core/post_processing_options.h"
#include "core/render_entity.h"
#include "core/render_entity_manager.h"
#include "core/service_locator.h"
#include "core/sparse_set.h"
#include "graphics/colour.h"
#include "graphics/decal.h"
#include "graphics/mesh_manager.h"
#include "graphics/point_light.h"
#include "graphics/texture_manager.h"
#include "maths/matrix4.h"
#include "maths/ray.h"
#include "maths/transform.h"
#include "maths/utils.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
#include "physics/physics_layers.h"
#include "physics/physics_system.h"
#include "utils/log.h"
#include "utils/string_map.h"

namespace ufps
{

class Scene
{
  public:
    struct Description
    {
        PostProcessingOptions post_processing_options;
        Colour ambient;
        std::vector<Entity::Description> entities;
    };

    Scene(const Description &description);

    auto add(EntityHandle handle) -> void;

    auto static_entities() const -> std::span<const EntityHandle>;

    auto dynamic_entities() const -> std::span<const EntityHandle>;

    auto all_entities() const;

    auto gun() const -> EntityHandle;

    auto &ambient_light(this auto &&self);

    auto &post_processing_options(this auto &&self);

    auto description() -> Description;

    auto remove(EntityHandle handle) -> void;

    auto decals() const -> std::span<const Decal>;

    auto add_decal(const Transform &transform, std::string_view decal_texture) -> void;

    auto check_and_reset_static_dirty() -> bool;

    auto set_is_static_dirty() -> void;

  private:
    PostProcessingOptions post_processing_options_;
    std::vector<EntityHandle> static_entities_;
    std::vector<EntityHandle> dynamic_entities_;
    EntityHandle gun_;
    Colour ambient_;
    std::vector<Decal> decals_;
    std::vector<Decal>::iterator next_decal_;
    bool decals_full_;
    bool is_static_dirty_;
};

auto &Scene::ambient_light(this auto &&self)
{
    return self.ambient_;
}

auto &Scene::post_processing_options(this auto &&self)
{
    return self.post_processing_options_;
}

inline auto Scene::all_entities() const
{
    return std::views::concat(static_entities(), dynamic_entities());
}

}
