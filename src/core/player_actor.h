#pragma once

#include "core/actor.h"
#include "core/camera.h"
#include "events/key.h"
#include "events/key_map.h"

namespace ufps
{
class PlayerActor : public Actor
{
  public:
    PlayerActor(Camera camera, const KeyMap &key_map);
    ~PlayerActor() override = default;

    auto update() -> void override;

  private:
    const KeyMap &key_map_;
};
}
