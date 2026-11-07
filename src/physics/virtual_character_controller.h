#pragma once

#include <chrono>

#include "core/clock.h"
#include "core/sparse_set.h"
#include "maths/quaternion.h"
#include "maths/spring.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_debug_renderer.h"
#include "physics/utils.h"

namespace ufps
{
class VirtualCharacterController : public ::JPH::CharacterContactListener
{
  public:
    VirtualCharacterController(::JPH::PhysicsSystem *ps);

    VirtualCharacterController(const VirtualCharacterController &) = delete;
    auto operator=(const VirtualCharacterController &) -> VirtualCharacterController & = delete;
    VirtualCharacterController(VirtualCharacterController &&) noexcept;
    auto operator=(VirtualCharacterController &&) -> VirtualCharacterController & = delete;

    auto update(std::chrono::milliseconds delta, const Vector3 &gravity) -> void;

    auto set_walk_direction(const Vector3 &walk_direction) -> void;

    auto jump() -> void;

    auto on_ground() const -> bool;

    auto in_air() const -> bool;

    auto jump_strength() const -> float;

    auto set_jump_strength(float jump_strength) -> void;

    auto debug_draw(PhysicsDebugRenderer &renderer) -> void;

    auto position() const -> Vector3;

    auto set_position(const Vector3 &position) -> void;

    auto rotation() const -> Quaternion;

    auto swap(VirtualCharacterController &other) noexcept -> void;

  protected:
    auto OnContactAdded(
        const ::JPH::CharacterVirtual *inCharacter,
        const ::JPH::BodyID &inBodyID2,
        const ::JPH::SubShapeID &inSubShapeID2,
        ::JPH::RVec3Arg inContactPosition,
        ::JPH::Vec3Arg inContactNormal,
        ::JPH::CharacterContactSettings &ioSettings) -> void override;

  private:
    ::JPH::PhysicsSystem *ps_;
    ::JPH::RefConst<::JPH::Shape> shape_;
    ::JPH::RefConst<::JPH::Shape> inner_shape_;
    ::JPH::Ref<::JPH::CharacterVirtual> character_;
    CharacterControllerCollisionObjectLayer object_layer_;
    Vector3 walk_direction_;
    bool do_jump_;
    float jump_strength_;
};

inline auto swap(VirtualCharacterController &a, VirtualCharacterController &b) noexcept -> void
{
    a.swap(b);
}

using VirtualCharacterControllerHandle = SparseSet<VirtualCharacterController>::handle_type;

}
