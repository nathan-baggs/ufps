#include "physics/rigid_body.h"

#include "maths/vector3.h"
#include "physics/jolt.h"

namespace ufps
{
RigidBody::RigidBody(::JPH::BodyID body_id, ::JPH::BodyInterface *body_interface)
    : body_id_{body_id}
    , body_interface_{body_interface}
{
}

auto RigidBody::position() const -> Vector3
{
    return to_native(body_interface_->GetPosition(body_id_));
}

}
