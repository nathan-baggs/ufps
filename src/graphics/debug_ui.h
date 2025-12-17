#pragma once

#include "events/mouse_button_event.h"
#include "graphics/scene.h"
#include "graphics/window.h"

namespace ufps
{

class DebugUI
{
  public:
    DebugUI(const Window &window);
    ~DebugUI();

    auto render(Scene &scene) const -> void;

    auto add_mouse_event(const MouseButtonEvent &evt) const -> void;

  private:
    const Window &window_;
};

}
