#include "core/entity.h"

#include <algorithm>
#include <span>
#include <string>
#include <vector>

#include "core/sub_mesh.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace
{
auto create_aabb(std::span<ufps::SubMesh> sub_meshes) -> ufps::AABB
{
    auto initial_aabb = ufps::AABB{
        .min = {std::numeric_limits<float>::max()},
        .max = {std::numeric_limits<float>::lowest()},
    };

    return std::ranges::fold_left(
        sub_meshes,
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
Entity::Entity(std::string name, std::vector<SubMesh> sub_meshes, Transform transform)
    : name_{std::move(name)}
    , sub_meshes_{std::move(sub_meshes)}
    , transform_{std::move(transform)}
    , aabb_{create_aabb(sub_meshes_)}
{
}

auto Entity::name() const -> std::string
{
    return name_;
}

auto Entity::sub_meshes() const -> std::span<const SubMesh>
{
    return sub_meshes_;
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

}
