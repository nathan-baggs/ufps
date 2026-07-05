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
    auto transform() const -> Transform;
    auto local_transform() const -> Transform;
    auto parent_transform() const -> Transform;
    auto set_local_transform(const Transform &transform) -> void;
    auto set_parent_transform(const Transform &transform) -> void;

  private:
    auto update_transforms(const Transform &local, const Transform &parent) -> void;

    ::JPH::BodyID body_id_;
    ::JPH::BodyInterface *body_interface_;
    ::JPH::RefConst<::JPH::Shape> original_shape_;
    Transform local_transform_;
    Transform parent_transform_;
    Vector3 applied_scale_;
};

}
