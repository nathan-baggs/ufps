#include "core/entity.h"

#include <algorithm>
#include <span>
#include <string>
#include <vector>

#include "core/render_entity.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace
{
auto create_aabb(std::span<ufps::RenderEntity> render_entities) -> ufps::AABB
{
    auto initial_aabb = ufps::AABB{
        .min = {std::numeric_limits<float>::max()},
        .max = {std::numeric_limits<float>::lowest()},
    };

    return std::ranges::fold_left(
        render_entities,
        initial_aabb,
        [](const auto &a, const auto &e)
        {
            return ufps::AABB{
                .min =
                    {
                        std::min(a.min.x, e.aabb().min.x),
                        std::min(a.min.y, e.aabb().min.y),
                        std::min(a.min.z, e.aabb().min.z),
                    },
                .max =
                    {
                        std::max(a.max.x, e.aabb().max.x),
                        std::max(a.max.y, e.aabb().max.y),
                        std::max(a.max.z, e.aabb().max.z),
                    },
            };
        });
}
}

namespace ufps
{
Entity::Entity(std::string name, std::vector<RenderEntity> render_entities, Transform transform)
    : name_{std::move(name)}
    , render_entities_{std::move(render_entities)}
    , transform_{std::move(transform)}
    , aabb_{create_aabb(render_entities_)}
{
}

auto Entity::name() const -> std::string
{
    return name_;
}

auto Entity::render_entities() const -> std::span<const RenderEntity>
{
    return render_entities_;
}

auto Entity::transform() const -> const Transform &
{
    return transform_;
}

auto Entity::set_transform(const Transform &transform) -> void
{
    transform_ = transform;
}

auto Entity::aabb() const -> const AABB &
{
    return aabb_;
}

auto Entity::description() const -> Entity::Description
{
    return {
        .name = name_,
        .transform = transform_,
        .aabb = aabb_,
    };
}

}
