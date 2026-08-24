#include "graphics/debug_layer.h"

#include "concurrency/concurrent_queue.h"
#include "graphics/colour.h"
#include "graphics/line_data.h"
#include "maths/vector3.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace ufps
{

auto DebugLayer::push_line(
    const ufps::Vector3 &start,
    const ufps::Vector3 &end,
    const ufps::Colour &colour,
    DebugLayerType type) -> void
{
    switch (type)
    {
        using enum DebugLayerType;

        case DEFAULT:
            default_lines_queue_.push({start, colour});
            default_lines_queue_.push({end, colour});
            break;
        case DEBUG:
            debug_lines_queue_.push({start, colour});
            debug_lines_queue_.push({end, colour});
            break;
    }
}

auto DebugLayer::push_cube(const Transform &transform, const Colour &colour, DebugLayerType type) -> void
{
    switch (type)
    {
        using enum DebugLayerType;
        case DEFAULT: default_cubes_queue_.push({transform, colour}); break;
        case DEBUG: default_cubes_queue_.push({transform, colour}); break;
    }
}

auto DebugLayer::push_aabb(const AABB &aabb, const Transform &transform, const Colour &colour, DebugLayerType type)
    -> void
{
    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        colour,
        type);

    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        type);

    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        type);
    push_line(
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        type);
}

auto DebugLayer::push_frustrum(const Camera &camera, const Colour &colour, DebugLayerType type) -> void
{
    const auto &proj = camera.data().projection;
    const auto &view = camera.data().view;
    const auto &cam_pos = camera.data().position;

    const auto inv_vp = ufps::Matrix4::invert(proj * view);

    constexpr auto ndc_far_corners = std::array<ufps::Vector4, 4>{{
        {-1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f},
    }};

    static constexpr auto debug_distance = 3.0f;

    auto far_corners = std::array<ufps::Vector3, 4>{};
    for (auto i = 0zu; i < 4zu; ++i)
    {
        const auto world_h = inv_vp * ndc_far_corners[i];
        const auto world_pos = ufps::Vector3(world_h) / world_h.w;
        const auto ray_dir = ufps::Vector3::normalise(world_pos - cam_pos);

        far_corners[i] = cam_pos + ray_dir * debug_distance;
    }

    for (auto i = 0zu; i < 4zu; ++i)
    {
        push_line(cam_pos, far_corners[i], colour, type);
    }

    for (auto i = 0zu; i < 4zu; ++i)
    {
        push_line(far_corners[i], far_corners[(i + 1) % 4], colour, type);
    }
}

auto DebugLayer::yield_lines(DebugLayerType type) -> std::queue<LineData>
{
    switch (type)
    {
        using enum DebugLayerType;

        case DEFAULT: return default_lines_queue_.yield();
        case DEBUG: return debug_lines_queue_.yield();
    }

    throw Exception("unknown type: {}", type);
}

auto DebugLayer::yield_cubes(DebugLayerType type) -> std::queue<CubeData>
{
    switch (type)
    {
        using enum DebugLayerType;

        case DEFAULT: return default_cubes_queue_.yield();
        case DEBUG: return debug_cubes_queue_.yield();
    }

    throw Exception("unknown type: {}", type);
}

}
