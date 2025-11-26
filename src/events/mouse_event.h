#pragma once

#include <string>

namespace ufps
{

class MouseEvent
{
  public:
    MouseEvent(float delta_x, float delta_y);

    auto delta_x() const -> float;
    auto delta_y() const -> float;

    auto to_string() const -> std::string;

  private:
    float delta_x_;
    float delta_y_;
};

}
