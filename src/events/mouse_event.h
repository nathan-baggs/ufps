#pragma once

#include <format>
#include <string>

namespace ufps
{

class MouseEvent
{
  public:
    constexpr MouseEvent(float delta_x, float delta_y);

    constexpr auto delta_x() const -> float;
    constexpr auto delta_y() const -> float;

    constexpr auto to_string() const -> std::string;

  private:
    float delta_x_;
    float delta_y_;
};

constexpr MouseEvent::MouseEvent(float delta_x, float delta_y)
    : delta_x_(delta_x)
    , delta_y_(delta_y)
{
}

constexpr auto MouseEvent::delta_x() const -> float
{
    return delta_x_;
}

constexpr auto MouseEvent::delta_y() const -> float
{
    return delta_y_;
}

constexpr auto MouseEvent::to_string() const -> std::string
{
    return std::format("MouseEvent {} {}", delta_x(), delta_y());
}

}
