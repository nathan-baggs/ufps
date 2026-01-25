#pragma once

#include <optional>

#include "core/entity.h"
#include "core/scene.h"
#include "events/mouse_button_event.h"
#include "graphics/mesh_manager.h"
#include "graphics/renderer.h"
#include "graphics/texture_manager.h"
#include "graphics/window.h"
#include "resources/resource_loader.h"

namespace ufps
{

class DebugRenderer : public Renderer
{
  public:
    DebugRenderer(
        const Window &window,
        ResourceLoader &resource_loader,
        TextureManager &texture_manager,
        MeshManager &mesh_manager);
    ~DebugRenderer();

    auto add_mouse_event(const MouseButtonEvent &evt) -> void;

    auto set_enabled(bool enabled) -> void;

  protected:
    auto post_render(Scene &scene) -> void override;

  private:
    bool enabled_;
    std::optional<MouseButtonEvent> click_;
    const Entity *selected_entity_;
};

}
