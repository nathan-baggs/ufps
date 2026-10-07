#pragma once

#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "maths/vector3.h"

namespace ufps
{

class Actor
{
  public:
    Actor(EntityHandle entity);
    virtual ~Actor() = default;
    Actor(const Actor &) = delete;
    auto operator=(const Actor &) -> Actor & = delete;
    Actor(Actor &&) = default;
    auto operator=(Actor &&) -> Actor & = default;

    virtual auto update(Duration delta) -> void = 0;

    auto &camera(this auto &&self);

    auto translate(const Vector3 &delta) -> void;

    auto entity() const -> EntityHandle;

  protected:
    CameraHandle camera_;
    EntityHandle entity_;
};

auto &Actor::camera(this auto &&self)
{
    return self.camera_;
}

}
