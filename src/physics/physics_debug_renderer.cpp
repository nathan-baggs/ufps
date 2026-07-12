#include "physics/physics_debug_renderer.h"

#include <ranges>
#include <vector>

#include "graphics/line_data.h"
#include "physics/jolt.h"

namespace ufps
{

auto PhysicsDebugRenderer::DrawLine(::JPH::RVec3Arg from, ::JPH::RVec3Arg to, ::JPH::ColorArg colour) -> void
{
    lines_.push_back({to_native(from), to_native(colour)});
    lines_.push_back({to_native(to), to_native(colour)});
}

auto PhysicsDebugRenderer::DrawTriangle(
    ::JPH::RVec3Arg v1,
    ::JPH::RVec3Arg v2,
    ::JPH::RVec3Arg v3,
    ::JPH::ColorArg colour,
    ECastShadow) -> void
{
    DrawLine(v1, v2, colour);
    DrawLine(v1, v3, colour);
    DrawLine(v2, v3, colour);
}

auto PhysicsDebugRenderer::DrawText3D(::JPH::RVec3Arg, const std::string_view &, ::JPH::ColorArg, float) -> void
{
}

auto PhysicsDebugRenderer::yield_lines() -> std::vector<LineData>
{
    auto copy = std::vector<LineData>{};
    std::ranges::swap(copy, lines_);
    return copy;
}

}
