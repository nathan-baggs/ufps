#pragma once

#include "core/actor.h"
#include "core/camera.h"
#include "events/input_map.h"
#include "events/key.h"
#include "physics/virtual_character_controller.h"

namespace ufps
{
class PlayerActor : public Actor
{
  public:
    PlayerActor(Camera camera, const InputMap &input_map, VirtualCharacterController &character_controller);
    ~PlayerActor() override = default;

    auto update() -> void override;

  private:
    const InputMap &input_map_;
    VirtualCharacterController &character_controller_;
};
}
