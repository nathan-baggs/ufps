#include "core/actor.h"

#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "maths/transform.h"
#include "maths/vector3.h"

namespace ufps
{

Actor::Actor(EntityHandle entity)
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
}

auto Actor::translate(const Vector3 &delta) -> void
{
    const auto &[em] = services<EntityManager>();
    const auto e = em[entity_];
    ensure(e, "entity missing");

    auto transform = e->transform();
    transform.position += delta;
    e->set_transform(transform);
}

auto Actor::entity() const -> EntityHandle
{
    return entity_;
}

}
