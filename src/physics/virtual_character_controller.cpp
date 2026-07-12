#include "physics/virtual_character_controller.h"

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

    auto settings = ::JPH::Ref{new ::JPH::CharacterVirtualSettings{}};
    settings->mShape = shape_;
    character_ =
        new ::JPH::CharacterVirtual(settings, ::JPH::Vec3::sZero(), ::JPH::Quat::sIdentity(), 0, std::addressof(ps));
}

auto VirtualCharacterController::update(std::chrono::milliseconds delta) -> void
{
    static auto temp_allocator = ::JPH::TempAllocatorImpl{10zu * 1024zu * 1024zu};

    character_->Update(
        1.0f / delta.count(),
        -character_->GetUp() * ps_.GetGravity().Length(),
        ps_.GetDefaultBroadPhaseLayerFilter(static_cast<::JPH::ObjectLayer>(PhysicsLayer::STATIC)),
        ps_.GetDefaultLayerFilter(static_cast<::JPH::ObjectLayer>(PhysicsLayer::STATIC)),
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

}
