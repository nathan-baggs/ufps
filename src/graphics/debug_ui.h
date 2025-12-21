#pragma once

#include <optional>

#include "core/entity.h"
#include "core/scene.h"
#include "events/mouse_button_event.h"
#include "graphics/window.h"

namespace ufps
{

class DebugUI
{
  public:
    DebugUI(const Window &window);
    ~DebugUI();

    auto render(Scene &scene) -> void;

    auto add_mouse_event(const MouseButtonEvent &evt) -> void;

  private:
    const Window &window_;
    std::optional<MouseButtonEvent> click_;
    const Entity *selected_entity_;
};

}
