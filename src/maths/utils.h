#pragma once

#include <optional>

#include "maths/aabb.h"
#include "maths/ray.h"
#include "maths/vector3.h"

namespace ufps
{

constexpr auto intersect(const Ray &ray, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2)
    -> std::optional<float>
{
    const auto edge1 = v1 - v0;
    const auto edge2 = v2 - v0;

    const auto h = Vector3::cross(ray.direction, edge2);
    const auto a = Vector3::dot(edge1, h);

    if (std::abs(a) < 1e-8f)
    {
        return {};
    }

    const auto f = 1.0f / a;
    const auto s = ray.origin - v0;
    const auto u = f * Vector3::dot(s, h);

    if (u < 0.0f || u > 1.0f)
    {
        return {};
    }

    const auto q = Vector3::cross(s, edge1);
    const auto v = f * Vector3::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
    {
        return {};
    }

    const auto t = f * Vector3::dot(edge2, q);
    return t > 1e-8f ? std::make_optional(t) : std::nullopt;
}

constexpr auto intersect(const Ray &ray, const AABB &aabb) -> std::optional<float>
{
    const auto inv_dir = Vector3{1.0f} / ray.direction;
    const auto t1 = (aabb.min - ray.origin) * inv_dir;
    const auto t2 = (aabb.max - ray.origin) * inv_dir;

    const auto tmin = std::max(std::max(std::min(t1.x, t2.x), std::min(t1.y, t2.y)), std::min(t1.z, t2.z));
    const auto tmax = std::min(std::min(std::max(t1.x, t2.x), std::max(t1.y, t2.y)), std::max(t1.z, t2.z));

    if (tmax < 0.0f || tmin > tmax)
    {
        return {};
    }

    return std::make_optional(tmin);
}

}
