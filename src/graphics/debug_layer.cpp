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

}
