#pragma once

#include <string>

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
    MouseButtonEvent(float x, float y, MouseButtonState state);

    auto x() const -> float;
    auto y() const -> float;
    auto state() const -> MouseButtonState;
    auto to_string() const -> std::string;

  private:
    float x_;
    float y_;
    MouseButtonState state_;
};

auto to_string(MouseButtonState obj) -> std::string;

}
