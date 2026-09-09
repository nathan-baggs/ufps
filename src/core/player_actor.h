#pragma once

#include "core/actor.h"
#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/scene.h"
#include "events/input_map.h"
#include "events/key.h"
#include "maths/spring.h"
#include "physics/virtual_character_controller.h"

namespace ufps
{
class PlayerActor : public Actor
{
  public:
    PlayerActor(
        EntityHandle entity,
        const InputMap &input_map,
        VirtualCharacterController &character_controller,
        const Scene &scene);
    ~PlayerActor() override = default;

    auto update(Duration delta) -> void override;

  private:
    const InputMap &input_map_;
    VirtualCharacterController &character_controller_;
    std::vector<std::tuple<Vector3, Vector3, Colour>> pew_pew_lines_;
    const Scene &scene_;
    Duration walk_sound_timer_;
    Duration shoot_timer_;
    Spring recoil_spring_;
};
}
