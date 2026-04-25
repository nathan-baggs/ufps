#pragma once

#include <format>
#include <string>

#include "utils/formatter.h"

namespace ufps
{

enum class MouseButtonState
{
    UP,
    DOWN
};

class MouseButtonEvent
{
  public:
    constexpr MouseButtonEvent(float x, float y, MouseButtonState state);

    constexpr auto x() const -> float;
    constexpr auto y() const -> float;
    constexpr auto state() const -> MouseButtonState;
    constexpr auto to_string() const -> std::string;

  private:
    float x_;
    float y_;
    MouseButtonState state_;
};

constexpr MouseButtonEvent::MouseButtonEvent(float x, float y, MouseButtonState state)
    : x_(x)
    , y_(y)
    , state_(state)
{
}

constexpr auto MouseButtonEvent::x() const -> float
{
    return x_;
}

constexpr auto MouseButtonEvent::y() const -> float
{
    return y_;
}

constexpr auto MouseButtonEvent::state() const -> MouseButtonState
{
    return state_;
}

constexpr auto MouseButtonEvent::to_string() const -> std::string
{
    return std::format("MouseButtonEvent {} {} {}", state(), x(), y());
}

}
