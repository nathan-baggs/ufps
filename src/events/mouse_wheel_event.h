#pragma once

#include <format>
#include <string>

namespace ufps
{

class MouseWheelEvent
{
  public:
    constexpr MouseWheelEvent(float delta_y);

    constexpr auto delta_y() const -> float;

    constexpr auto to_string() const -> std::string;

  private:
    float delta_y_;
};

constexpr MouseWheelEvent::MouseWheelEvent(float delta_y)
    : delta_y_{delta_y}
{
}

constexpr auto MouseWheelEvent::delta_y() const -> float
{
    return delta_y_;
}

constexpr auto MouseWheelEvent::to_string() const -> std::string
{
    return std::format("MouseWheelEvent {}", delta_y_);
}

}
