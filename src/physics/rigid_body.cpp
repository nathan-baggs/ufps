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
    , original_shape_{body_interface_->GetShape(body_id_)}
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
    const auto new_transform = Transform{Matrix4{transform} * Matrix4{local_transform_}};
    const auto scale_changed = transform.scale != parent_transform_.scale;

    parent_transform_ = transform;

    body_interface_->SetPositionAndRotation(
        body_id_, to_jolt(new_transform.position), to_jolt(new_transform.rotation), ::JPH::EActivation::Activate);

    if (scale_changed)
    {
        const auto jolt_scale = to_jolt(new_transform.scale);

        const auto scaled_result = original_shape_->ScaleShape(jolt_scale);
        if (scaled_result.HasError())
        {
            throw Exception("scale error: {}", scaled_result.GetError());
        }

        body_interface_->SetShape(body_id_, scaled_result.Get(), false, ::JPH::EActivation::Activate);
    }
}
}
