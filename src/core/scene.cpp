#include "core/scene.h"

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

Scene::Scene(const Description &description)
    : post_processing_options_{description.post_processing_options}
    , entities_{}
    , gun_{}
    , ambient_{description.ambient}
    , decals_{100zu}
    , next_decal_{std::ranges::begin(decals_)}
    , decals_full_{false}
{
    auto &&[em, rem, ps, lm, cm] =
        services<EntityManager, RenderEntityManager, PhysicsSystem, LightManager, CameraManager>();

    auto lookup = StringMap<EntityHandle>{};

    for (const auto &entity_description : description.entities)
    {
        const auto new_entity_handle = em.insert({entity_description.name, {}, entity_description.transform});

        auto new_entity = em[new_entity_handle];
        new_entity->set_emissive_strength(entity_description.emissive_strength);

        for (const auto &group_name : entity_description.render_entities)
        {
            new_entity->add_render_entities(rem[group_name]);
        }

        for (const auto &rb_description : entity_description.rigid_bodies)
        {
            const auto rb = ps.create_rigid_body(rb_description, new_entity_handle);
            new_entity->add_rigid_body(rb);
        }

        if (entity_description.can_intersect_ray)
        {
            new_entity->set_ray_intersect_rigid_bodies(
                ps.create_meshes(BroadPhaseLayer::STATIC, ObjectLayer::LEVEL_GEOMETRY, new_entity_handle));
        }

        if (entity_description.camera)
        {
            const auto camera_handle = cm.insert(*entity_description.camera);
            new_entity->set_camera(camera_handle);
        }

        if (entity_description.light)
        {
            const auto light_handle = lm.insert(*entity_description.light);
            new_entity->set_light(light_handle);
        }

        if (new_entity->name() == "gun")
        {
            gun_ = new_entity_handle;
        }
        else
        {
            add(new_entity_handle);
        }

        lookup[entity_description.name] = new_entity_handle;
    }

    for (const auto &entity_description : description.entities)
    {
        const auto &entity = lookup[entity_description.name];

        for (const auto &child : entity_description.children)
        {
            if (em[entity]->name() == "player" && child == "gun" && gun_)
            {
                em[entity]->add_child(gun_);
                continue;
            }

            const auto child_handle = lookup.find(child);
            ensure(child_handle != std::ranges::cend(lookup), "child {} not found", child);

            em[entity]->add_child(child_handle->second);
        }
    }
}

auto Scene::add(EntityHandle handle) -> void
{
    entities_.push_back(handle);
}

auto Scene::entities() const -> std::span<const EntityHandle>
{
    return entities_;
}

auto Scene::gun() const -> EntityHandle
{
    return gun_;
}

auto Scene::description() -> Description
{
    const auto &[em, cm] = services<EntityManager, CameraManager>();

    auto entities = entities_ | std::views::filter([&](auto &e) { return !!em[e]; }) |
                    std::views::transform([&](auto e) { return em[e]->description(); }) |
                    std::ranges::to<std::vector>();

    if (const auto gun = em[gun_]; gun)
    {
        entities.push_back(gun->description());
    }

    return {
        .post_processing_options = post_processing_options_,
        .ambient = ambient_,
        .entities = std::move(entities),
    };
}

auto Scene::remove(EntityHandle handle) -> void
{
    const auto iter = std::ranges::find(entities_, handle);
    contract_assert(iter != std::ranges::cend(entities_));

    const auto &[ps, cm, em] = services<PhysicsSystem, CameraManager, EntityManager>();

    const auto entity = em[handle];
    contract_assert(entity);

    ps.remove(entity->rigid_bodies());
    ps.remove(entity->ray_intersect_rigid_bodies());

    if (entity->camera())
    {
        cm.remove(entity->camera());
    }

    entities_.erase(iter);
}

auto Scene::decals() const -> std::span<const Decal>
{
    const auto begin = std::ranges::cbegin(decals_);
    const auto end = decals_full_ ? std::ranges::cend(decals_) : next_decal_;

    return {begin, end};
}

auto Scene::add_decal(const Transform &transform, std::string_view decal_texture) -> void
{
    const auto &tm = service<TextureManager>();

    const auto mat = Matrix4{transform};
    const auto pos = transform.position;
    const auto normal = Vector3::normalise({mat[4], mat[5], mat[6]});

    if (std::ranges::none_of(
            decals_,
            [pos, normal](const auto &mat)
            {
                const auto decal_pos = Vector3{mat[12], mat[13], mat[14]};
                const auto decal_normal = Vector3::normalise({mat[4], mat[5], mat[6]});

                const auto distance = Vector3::distance(pos, decal_pos);
                const auto normal_alignment = Vector3::dot(normal, decal_normal);

                return distance < 0.01f && normal_alignment > 0.95f;
            },
            &Decal::transform))
    {
        *next_decal_ = Decal{transform, tm.bindless_handle(decal_texture)};
        ++next_decal_;

        if (next_decal_ == std::ranges::cend(decals_))
        {
            decals_full_ = true;
            next_decal_ = std::ranges::begin(decals_);
        }
    }
}

}
