#include "events/mouse_button_event.h"

#include <format>
#include <string>

#include "utils/formatter.h"

namespace ufps
{

MouseButtonEvent::MouseButtonEvent(float x, float y, MouseButtonState state)
    : x_(x)
    , y_(y)
    , state_(state)
{
}

auto MouseButtonEvent::x() const -> float
{
    return x_;
}
auto MouseButtonEvent::y() const -> float
{
    return y_;
}
auto MouseButtonEvent::state() const -> MouseButtonState
{
    return state_;
}

auto MouseButtonEvent::to_string() const -> std::string
{
    return std::format("MouseButtonEvent {} {} {}", state(), x(), y());
}

auto to_string(MouseButtonState obj) -> std::string
{
    switch (obj)
    {
        using enum ufps::MouseButtonState;
        case UP: return "UP";
        case DOWN: return "DOWN";
        default: return "UNKNOWN";
    }
}

}
