#pragma once

#include "core/actor.h"
#include "core/camera.h"
#include "events/key_map.h"

namespace ufps
{
class FlyCamActor : public Actor
{
  public:
    FlyCamActor(Camera camera, const KeyMap &key_map);
    ~FlyCamActor() override = default;

    auto update() -> void override;

  private:
    const KeyMap &key_map_;
};
}
