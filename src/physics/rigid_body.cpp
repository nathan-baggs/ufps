#include "physics/rigid_body.h"

#include "Jolt/Physics/EActivation.h"
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

auto RigidBody::set_transform(const Transform &transform) -> void
{
    body_interface_->SetPositionAndRotation(
        body_id_, to_jolt(transform.position), to_jolt(transform.rotation), ::JPH::EActivation::Activate);
}

}
