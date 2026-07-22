#pragma once

#include "core/actor.h"
#include "core/camera.h"
#include "events/input_map.h"

namespace ufps
{
class FlyCamActor : public Actor
{
  public:
    FlyCamActor(Camera camera, const InputMap &input_map);
    ~FlyCamActor() override = default;

    auto update() -> void override;

  private:
    const InputMap &input_map_;
};
}
