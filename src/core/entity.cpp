#include "core/entity.h"

#include <ranges>

#include "core/camera_manager.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "maths/transform.h"
#include "physics/physics_system.h"

namespace ufps
{

auto Entity::description() const -> Entity::Description
{
    const auto &[em, rem, ps, cm] = services<EntityManager, RenderEntityManager, PhysicsSystem, CameraManager>();

    auto render_entities = render_entities_ | std::views::filter([&](auto e) { return !!rem[e]; }) |
                           std::views::transform([&](auto e) { return std::string{rem[e]->group_name()}; }) |
                           std::ranges::to<std::vector>();
    std::ranges::sort(render_entities);
    const auto [first, last] = std::ranges::unique(render_entities);
    render_entities.erase(first, last);

    const auto camera = cm[camera_];

    return {
        .name = name_,
        .emissive_strength = emissive_strength_,
        .transform = transform_,
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
        .camera = camera.transform([](const auto &e) { return e.description(); })};
}

auto Entity::set_transform(const Transform &transform) -> void
{
    auto &&[em, ps, cm] = services<EntityManager, PhysicsSystem, CameraManager>();

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
