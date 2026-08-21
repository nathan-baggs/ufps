#include "core/entity.h"

#include <ranges>

#include "core/camera_manager.h"
#include "core/entity_manager.h"
#include "core/light_manager.h"
#include "core/service_locator.h"
#include "maths/transform.h"
#include "physics/physics_system.h"

namespace ufps
{

Entity::Entity(std::string name, std::span<const RenderEntityHandle> render_entities, Transform transform)
    : name_{std::move(name)}
    , render_entities_{std::ranges::cbegin(render_entities), std::ranges::cend(render_entities)}
    , rigid_bodies_{}
    , light_{}
    , local_transform_{std::move(transform)}
    , parent_transform_{{}, {1.0f}, {}}
    , transform_{parent_transform_ * local_transform_}
    , aabb_{create_aabb(render_entities_)}
    , emissive_strength_{1.0f}
{
}

auto Entity::name() const -> std::string_view
{
    return name_;
}

auto Entity::set_name(std::string name) -> void
{
    name_ = std::move(name);
}

auto Entity::render_entities() const -> std::span<const RenderEntityHandle>
{
    return render_entities_;
}

auto Entity::add_render_entities(std::span<const RenderEntityHandle> render_entities) -> void
{
    render_entities_.append_range(render_entities);
    aabb_ = create_aabb(render_entities_);
}

auto Entity::remove_render_entity(RenderEntityHandle handle) -> void
{
    std::erase(render_entities_, handle);
}

auto Entity::transform() const -> const Transform &
{
    return transform_;
}

auto Entity::local_transform() const -> const Transform &
{
    return local_transform_;
}

auto Entity::parent_transform() const -> const Transform &
{
    return parent_transform_;
}

auto Entity::aabb() const -> const AABB &
{
    return aabb_;
}

auto Entity::emissive_strength() const -> float
{
    return emissive_strength_;
}

auto Entity::set_emissive_strength(float strength) -> void
{
    emissive_strength_ = strength;
}

auto Entity::add_rigid_body(RigidBodyHandle handle) -> void
{
    rigid_bodies_.push_back(handle);
    service<PhysicsSystem>().rigid_body(handle)->set_parent_transform(transform_);
}

auto Entity::rigid_bodies() const -> std::span<const RigidBodyHandle>
{
    return rigid_bodies_;
}

auto Entity::light() const -> LightHandle
{
    return light_;
}

auto Entity::set_light(LightHandle handle) -> void
{
    light_ = handle;
}

auto Entity::set_parent_transform(const Transform &transform) -> void
{
    update_transforms(local_transform_, transform);
}

auto Entity::children() -> std::span<const EntityHandle>
{
    return children_;
}

auto Entity::description() const -> Entity::Description
{
    const auto &[em, rem, ps, cm, lm] =
        services<EntityManager, RenderEntityManager, PhysicsSystem, CameraManager, LightManager>();

    auto render_entities = render_entities_ | std::views::filter([&](auto e) { return !!rem[e]; }) |
                           std::views::transform([&](auto e) { return std::string{rem[e]->group_name()}; }) |
                           std::ranges::to<std::vector>();
    std::ranges::sort(render_entities);
    const auto [first, last] = std::ranges::unique(render_entities);
    render_entities.erase(first, last);

    const auto camera = cm[camera_];
    const auto light = lm[light_];

    log::debug("{} {}", name_, std::ranges::size(children_));

    return {
        .name = name_,
        .emissive_strength = emissive_strength_,
        .transform = local_transform_,
        .aabb = aabb_,
        .rigid_bodies = rigid_bodies_ | std::views::transform([&](auto e) { return ps.rigid_body(e); }) |
                        std::views::filter([](const auto &e) { return !!e; }) |
                        std::views::transform([](const auto &e) { return e->description(); }) |
                        std::ranges::to<std::vector>(),
        .render_entities = std::move(render_entities),
        .children = children_ | std::views::transform([&](auto e) { return em[e]; }) |
                    std::views::filter([](const auto &e) { return !!e; }) |
                    std::views::transform([](const auto &e) { return std::string{e->name()}; }) |
                    std::ranges::to<std::vector>(),
        .camera = camera.transform([](const auto &e) { return e.description(); }),
        .light = light.transform([](const auto &e) { return e; }),
    };
}

auto Entity::set_transform(const Transform &transform) -> void
{
    auto &&[em, ps, cm, lm] = services<EntityManager, PhysicsSystem, CameraManager, LightManager>();

    update_transforms(transform, parent_transform_);

    for (auto &handle : children_)
    {
        const auto child = em[handle];
        child->set_parent_transform(transform_);
    }

    rigid_bodies_ =
        rigid_bodies_ | std::views::filter([&](auto e) { return !!ps.rigid_body(e); }) | std::ranges::to<std::vector>();

    for (const auto handle : rigid_bodies_)
    {
        ps.rigid_body(handle)->set_parent_transform(transform_);
    }

    if (camera_)
    {
        const auto camera = cm[camera_];
        contract_assert(camera);

        camera->set_parent_transform(transform_);
    }

    if (light_)
    {
        const auto light = lm[light_];
        contract_assert(light);

        light->position = transform_.position;
    }
}

auto Entity::update_transforms(const Transform &local, const Transform &parent) -> void
{
    transform_ = parent * local;
    local_transform_ = local;
    parent_transform_ = parent;

    auto &&[em, ps] = services<EntityManager, PhysicsSystem>();

    for (auto handle : children_)
    {
        const auto child = em[handle];
        child->set_parent_transform(transform_);
    }
}

auto Entity::add_child(EntityHandle child) -> void
{
    auto &em = service<EntityManager>();

    const auto entity = em[child];
    contract_assert(entity);

    entity->set_parent_transform(transform_);

    children_.push_back(child);
}

auto Entity::camera() const -> CameraHandle
{
    return camera_;
}

auto Entity::set_camera(CameraHandle handle) -> void
{
    camera_ = handle;
}

}
