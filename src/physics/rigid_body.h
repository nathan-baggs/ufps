#pragma once

#include "maths/transform.h"
#include "maths/vector3.h"
#include "physics/jolt.h"

namespace ufps
{

class RigidBody
{
  public:
    RigidBody(::JPH::BodyID body_id, ::JPH::BodyInterface *body_interface);

    RigidBody(const RigidBody &) = delete;
    auto operator=(const RigidBody &) -> RigidBody & = delete;
    RigidBody(RigidBody &&) = default;
    auto operator=(RigidBody &&) -> RigidBody & = default;

    auto position() const -> Vector3;
    auto set_transform(const Transform &transform) -> void;

  private:
    ::JPH::BodyID body_id_;
    ::JPH::BodyInterface *body_interface_;
};

}
