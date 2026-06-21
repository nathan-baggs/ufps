#include "physics/rigid_body.h"

#include "Jolt/Physics/EActivation.h"
#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "utils/exception.h"
#include "utils/log.h"

namespace ufps
{
RigidBody::RigidBody(::JPH::BodyID body_id, ::JPH::BodyInterface *body_interface)
    : body_id_{body_id}
    , body_interface_{body_interface}
    , local_transform_{{}, {1.0f}, {}}
    , parent_transform_{{}, {1.0f}, {}}
{
}

auto RigidBody::position() const -> Vector3
{
    return to_native(body_interface_->GetPosition(body_id_));
}

auto RigidBody::set_parent_transform(const Transform &transform) -> void
{
    const auto scale_amount = Vector3{1.0f} + (transform.scale - parent_transform_.scale);

    parent_transform_ = transform;

    const auto new_transform = Transform{Matrix4{parent_transform_} * Matrix4{local_transform_}};

    body_interface_->SetPositionAndRotation(
        body_id_, to_jolt(new_transform.position), to_jolt(new_transform.rotation), ::JPH::EActivation::Activate);

    log::debug("scale: {}", scale_amount);
    const auto jolt_scale = to_jolt(scale_amount);

    if (!jolt_scale.IsNearZero())
    {
        const auto shape = body_interface_->GetShape(body_id_);
        const auto scaled_result = shape->ScaleShape(jolt_scale);
        if (scaled_result.HasError())
        {
            throw Exception("scale error: {}", scaled_result.GetError());
        }

        body_interface_->SetShape(body_id_, scaled_result.Get(), false, ::JPH::EActivation::Activate);
    }
}
}
