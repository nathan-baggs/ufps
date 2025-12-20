#pragma once

#include <optional>
#include <ranges>
#include <vector>

#include "core/camera.h"
#include "core/entity.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_manager.h"
#include "maths/ray.h"
#include "maths/utils.h"
#include "maths/vector4.h"

namespace ufps
{

struct IntersctionResult
{
    const Entity *entity;
    Vector3 position;
};

struct Scene
{
    constexpr auto intersect_ray(const Ray &ray) const -> std::optional<IntersctionResult>
    {
        auto result = std::optional<IntersctionResult>{};
        auto min_distance = std::numeric_limits<float>::max();

        for (const auto &entity : entities)
        {
            const auto inv_transform = Matrix4::invert(entity.transform);
            const auto transformed_ray =
                Ray{inv_transform * Vector4{ray.origin, 1.0f}, inv_transform * Vector4{ray.direction, 0.0f}};

            for (const auto &indices : std::views::chunk(entity.mesh_view.indices, 3))
            {
                const auto v0 = entity.mesh_view.vertices[indices[0]].position;
                const auto v1 = entity.mesh_view.vertices[indices[1]].position;
                const auto v2 = entity.mesh_view.vertices[indices[2]].position;

                if (const auto distance = intersect(transformed_ray, v0, v1, v2); distance)
                {
                    const auto intersection_point = transformed_ray.origin + transformed_ray.direction * (*distance);

                    if (!result && (*distance < min_distance))
                    {
                        result = IntersctionResult{.entity = &entity, .position = intersection_point};
                        min_distance = *distance;
                    }
                }
            }
        }

        return result;
    }

    std::vector<Entity> entities;
    MeshManager &mesh_manager;
    MaterialManager &material_manager;
    Camera camera;
};

}
