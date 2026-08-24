#pragma once

#include "concurrency/concurrent_queue.h"
#include "core/camera.h"
#include "graphics/colour.h"
#include "graphics/line_data.h"
#include "maths/aabb.h"
#include "maths/matrix4.h"
#include "maths/transform.h"
#include "maths/vector3.h"

namespace ufps
{

enum class DebugLayerType
{
    DEFAULT,
    DEBUG,
};

struct CubeData
{
    Transform transform;
    Colour colour;
};

class DebugLayer
{
  public:
    auto push_line(
        const Vector3 &start,
        const Vector3 &end,
        const Colour &colour,
        DebugLayerType type = DebugLayerType::DEBUG) -> void;

    auto push_cube(const Transform &transform, const Colour &colour, DebugLayerType type = DebugLayerType::DEBUG)
        -> void;

    auto push_aabb(
        const AABB &aabb,
        const Transform &transform,
        const Colour &colour,
        DebugLayerType type = DebugLayerType::DEBUG) -> void;

    auto push_frustrum(const Camera &camera, const Colour &colour, DebugLayerType type = DebugLayerType::DEBUG) -> void;

    auto yield_lines(DebugLayerType type) -> std::queue<LineData>;

    auto yield_cubes(DebugLayerType type) -> std::queue<CubeData>;

  private:
    ConcurrentQueue<LineData> default_lines_queue_;
    ConcurrentQueue<LineData> debug_lines_queue_;
    ConcurrentQueue<CubeData> default_cubes_queue_;
    ConcurrentQueue<CubeData> debug_cubes_queue_;
};

}
