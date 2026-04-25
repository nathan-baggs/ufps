#pragma once

#include <algorithm>
#include <span>

#include "core/render_entity.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace ufps
{

constexpr auto create_aabb(std::span<RenderEntity> render_entities) -> AABB
{
    auto initial_aabb = AABB{
        .min = {std::numeric_limits<float>::max()},
        .max = {std::numeric_limits<float>::lowest()},
    };

    return std::ranges::fold_left(
        render_entities,
        initial_aabb,
        [](const auto &a, const auto &e)
        {
            return AABB{
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
