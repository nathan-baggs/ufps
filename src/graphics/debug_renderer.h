#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "core/entity.h"
#include "core/scene.h"
#include "events/mouse_button_event.h"
#include "graphics/line_data.h"
#include "graphics/point_light.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "resources/resource_loader.h"

namespace ufps
{

class DebugRenderer : public Renderer
{
  public:
    DebugRenderer(const Window &window, ResourceLoader &resource_loader);
    ~DebugRenderer();

    auto add_mouse_event(const MouseButtonEvent &evt) -> void;

    auto set_enabled(bool enabled) -> void;

  protected:
    auto post_render(Scene &scene, const Camera &camera) -> void override;

  private:
    auto draw_post_processing_window(Scene &scene) -> void;
    auto draw_scene(Scene &scene, const Camera &camera) -> void;
    auto draw_logs() -> void;
    auto draw_bloom_mips() -> void;
    auto draw_render_targets() -> void;
    auto draw_metrics() -> void;
    auto draw_inspector(Scene &scene) -> void;
    auto draw_gizmo(const Camera &camera) -> void;

    bool enabled_;
    bool snap_enabled_;
    std::optional<MouseButtonEvent> click_;
    std::variant<std::monostate, EntityHandle, LightHandle, RigidBodyHandle> selected_;
    std::vector<LineData> debug_lines_;
    MultiBuffer<PersistentBuffer> debug_line_buffer_;
    Program debug_line_program_;
    Program debug_light_program_;
};

}
