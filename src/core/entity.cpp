#include "core/entity.h"

#include <ranges>

#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "maths/transform.h"
#include "physics/physics_system.h"

namespace ufps
{

auto Entity::set_transform(const Transform &transform) -> void
{
    auto &&[em, ps] = services<EntityManager, PhysicsSystem>();

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
}

}
