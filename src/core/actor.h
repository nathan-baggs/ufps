#pragma once

#include "core/camera.h"

namespace ufps
{

class Actor
{
  public:
    constexpr Actor(Camera camera);
    virtual ~Actor() = default;
    Actor(const Actor &) = delete;
    auto operator=(const Actor &) -> Actor & = delete;
    Actor(Actor &&) = default;
    auto operator=(Actor &&) -> Actor & = default;

    virtual auto update() -> void = 0;

    constexpr auto &camera(this auto &&self);

  protected:
    Camera camera_;
};

constexpr Actor::Actor(Camera camera)
    : camera_{std::move(camera)}
{
}

constexpr auto &Actor::camera(this auto &&self)
{
    return self.camera_;
}
}
