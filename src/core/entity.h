#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "core/render_entity.h"
#include "core/render_entity_manager.h"
#include "core/service_locator.h"
#include "core/utils.h"
#include "maths/aabb.h"
#include "maths/transform.h"
#include "physics/physics_system.h"

namespace ufps
{

class Entity
{
    using EntityHandle = SparseSet<Entity>::handle_type;

  public:
    struct Description
    {
        std::string name;
        float emissive_strength;
        Transform transform;
        AABB aabb;
        std::vector<RigidBody::Description> rigid_bodies;
    };

    constexpr Entity(std::string name, std::vector<RenderEntityHandle> render_entities, Transform transform);

    constexpr auto name() const -> std::string_view;
    constexpr auto render_entities() const -> std::span<const RenderEntityHandle>;
    constexpr auto add_render_entities(std::span<const RenderEntityHandle> render_entities);
    constexpr auto transform() const -> const Transform &;
    constexpr auto parent_transform() const -> const Transform &;
    auto set_transform(const Transform &transform) -> void;
    constexpr auto aabb() const -> const AABB &;
    constexpr auto description() const -> Description;
    constexpr auto emissive_strength() const -> float;
    constexpr auto set_emissive_strength(float strength) -> void;
    constexpr auto add_rigid_body(RigidBodyHandle handle);
    constexpr auto rigid_bodies() const -> std::span<const RigidBodyHandle>;
    auto add_child(EntityHandle child) -> void;

  private:
    auto update_transforms(const Transform &local, const Transform &parent) -> void;
    constexpr auto set_parent_transform(const Transform &transform) -> void;

    std::string name_;
    std::vector<RenderEntityHandle> render_entities_;
    std::vector<RigidBodyHandle> rigid_bodies_;
    Transform local_transform_;
    Transform parent_transform_;
    Transform transform_;
    AABB aabb_;
    float emissive_strength_;
    std::vector<EntityHandle> children_;
};

constexpr Entity::Entity(std::string name, std::vector<RenderEntityHandle> render_entities, Transform transform)
    : name_{std::move(name)}
    , render_entities_{std::move(render_entities)}
    , rigid_bodies_{}
    , local_transform_{std::move(transform)}
    , parent_transform_{{}, {1.0f}, {}}
    , transform_{parent_transform_ * local_transform_}
    , aabb_{create_aabb(render_entities_)}
    , emissive_strength_{1.0f}
{
}

constexpr auto Entity::name() const -> std::string_view
{
    return name_;
}

constexpr auto Entity::render_entities() const -> std::span<const RenderEntityHandle>
{
    return render_entities_;
}

constexpr auto Entity::add_render_entities(std::span<const RenderEntityHandle> render_entities)
{
    render_entities_.append_range(render_entities);
    aabb_ = create_aabb(render_entities_);
}

constexpr auto Entity::transform() const -> const Transform &
{
    return transform_;
}

constexpr auto Entity::parent_transform() const -> const Transform &
{
    return parent_transform_;
}

constexpr auto Entity::aabb() const -> const AABB &
{
    return aabb_;
}

constexpr auto Entity::description() const -> Entity::Description
{
    return {
        .name = name_,
        .emissive_strength = emissive_strength_,
        .transform = transform_,
        .aabb = aabb_,
        .rigid_bodies = rigid_bodies_ |
                        std::views::transform(
                            [](auto e)
                            {
                                auto &physics = service<PhysicsSystem>();
                                return physics.rigid_body(e);
                            }) |
                        std::views::filter([](const auto &e) { return !!e; }) |
                        std::views::transform([](const auto &e) { return e->description(); }) |
                        std::ranges::to<std::vector>(),
    };
}

constexpr auto Entity::emissive_strength() const -> float
{
    return emissive_strength_;
}

constexpr auto Entity::set_emissive_strength(float strength) -> void
{
    emissive_strength_ = strength;
}

constexpr auto Entity::add_rigid_body(RigidBodyHandle handle)
{
    rigid_bodies_.push_back(handle);
    service<PhysicsSystem>().rigid_body(handle)->set_parent_transform(transform_);
}

constexpr auto Entity::rigid_bodies() const -> std::span<const RigidBodyHandle>
{
    return rigid_bodies_;
}

constexpr auto Entity::set_parent_transform(const Transform &transform) -> void
{
    update_transforms(local_transform_, transform);
}

}
