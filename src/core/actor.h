#pragma once

#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"

namespace ufps
{

class Actor
{
  public:
    constexpr Actor(EntityHandle entity);
    virtual ~Actor() = default;
    Actor(const Actor &) = delete;
    auto operator=(const Actor &) -> Actor & = delete;
    Actor(Actor &&) = default;
    auto operator=(Actor &&) -> Actor & = default;

    virtual auto update() -> void = 0;

    constexpr auto &camera(this auto &&self);

  protected:
    CameraHandle camera_;
    EntityHandle entity_;
};

constexpr Actor::Actor(EntityHandle entity)
    : camera_{}
    , entity_{entity}
{
    const auto &[em, cm] = services<EntityManager, CameraManager>();
    const auto e = em[entity_];
    ensure(e, "entity missing");

    const auto camera_handle = e->camera();
    ensure(!!camera_handle, "no camera attached to entity");

    const auto cam = cm[camera_handle];
    ensure(cam, "camera missing");

    camera_ = camera_handle;

    // always reset entity to origin, ignore anything saved in the scene file
    e->set_transform({{0.0f, 2.0f, 0.0f}, {1.0f}, {}});
}

constexpr auto &Actor::camera(this auto &&self)
{
    return self.camera_;
}
}
