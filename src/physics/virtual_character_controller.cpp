#include "physics/virtual_character_controller.h"

#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_layers.h"

namespace ufps
{

VirtualCharacterController::VirtualCharacterController(::JPH::PhysicsSystem &ps)
    : ps_{ps}
    , shape_{}
    , character_{}
{
    shape_ = ::JPH::RotatedTranslatedShapeSettings(
                 ::JPH::Vec3(0, 0.5f * 1.35f + 0.3f, 0),
                 ::JPH::Quat::sIdentity(),
                 new ::JPH::CapsuleShape(0.5f * 1.35f, 0.3f))
                 .Create()
                 .Get();

    inner_shape_ = ::JPH::RotatedTranslatedShapeSettings(
                       ::JPH::Vec3(0, 0.5f * 1.35f + 0.3f, 0),
                       ::JPH::Quat::sIdentity(),
                       new ::JPH::CapsuleShape(0.5f * 1.35f, 0.3f * 0.9))
                       .Create()
                       .Get();

    auto settings = ::JPH::Ref{new ::JPH::CharacterVirtualSettings{}};
    settings->mShape = shape_;
    settings->mInnerBodyShape = inner_shape_;
    settings->mInnerBodyLayer = static_cast<::JPH::ObjectLayer>(PhysicsLayer::DYNAMIC);
    character_ =
        new ::JPH::CharacterVirtual(settings, ::JPH::Vec3::sZero(), ::JPH::Quat::sIdentity(), 0, std::addressof(ps));

    character_->SetListener(this);
}

auto VirtualCharacterController::update(std::chrono::milliseconds delta, const Vector3 &gravity) -> void
{
    static auto temp_allocator = ::JPH::TempAllocatorImpl{10zu * 1024zu * 1024zu};

    const auto jolt_delta = 1.0f / delta.count();

    auto new_velocity = Vector3{0.0f, 0.0f, -0.5f};
    new_velocity += gravity * jolt_delta;

    character_->SetLinearVelocity(to_jolt(new_velocity));

    character_->Update(
        jolt_delta,
        -character_->GetUp() * ps_.GetGravity().Length(),
        ps_.GetDefaultBroadPhaseLayerFilter(static_cast<::JPH::ObjectLayer>(PhysicsLayer::DYNAMIC)),
        ps_.GetDefaultLayerFilter(static_cast<::JPH::ObjectLayer>(PhysicsLayer::DYNAMIC)),
        {},
        {},
        temp_allocator);
}

auto VirtualCharacterController::debug_draw(PhysicsDebugRenderer &renderer) -> void
{
    character_->GetShape()->Draw(
        std::addressof(renderer),
        character_->GetCenterOfMassTransform(),
        ::JPH::Vec3::sOne(),
        ::JPH::Color::sOrange,
        false,
        true);
}

auto VirtualCharacterController::position() const -> Vector3
{
    return to_native(character_->GetPosition());
}

auto VirtualCharacterController::rotation() const -> Quaternion
{
    return to_native(character_->GetRotation());
}

auto VirtualCharacterController::OnContactAdded(
    [[maybe_unused]] const ::JPH::CharacterVirtual *inCharacter,
    [[maybe_unused]] const ::JPH::BodyID &inBodyID2,
    [[maybe_unused]] const ::JPH::SubShapeID &inSubShapeID2,
    [[maybe_unused]] ::JPH::RVec3Arg inContactPosition,
    [[maybe_unused]] ::JPH::Vec3Arg inContactNormal,
    [[maybe_unused]] ::JPH::CharacterContactSettings &ioSettings) -> void
{
    log::debug("here");
}

}
