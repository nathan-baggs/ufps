#pragma once

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

  private:
    ::JPH::BodyID body_id_;
    ::JPH::BodyInterface *body_interface_;
};

}
