#pragma once

#include "concurrency/concurrent_queue.h"
#include "graphics/colour.h"
#include "graphics/line_data.h"
#include "maths/vector3.h"

namespace ufps
{

enum class DebugLayerType
{
    DEFAULT,
    DEBUG,
};

class DebugLayer
{
  public:
    auto push_line(
        const ufps::Vector3 &start,
        const ufps::Vector3 &end,
        const ufps::Colour &colour,
        DebugLayerType type = DebugLayerType::DEBUG) -> void;

    auto yield_lines(DebugLayerType type) -> std::queue<LineData>;

  private:
    ConcurrentQueue<LineData> default_lines_queue_;
    ConcurrentQueue<LineData> debug_lines_queue_;
};

}
