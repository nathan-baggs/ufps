#pragma once

#include <chrono>

#include "maths/quaternion.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_debug_renderer.h"

namespace ufps
{
class VirtualCharacterController
{
  public:
    VirtualCharacterController(::JPH::PhysicsSystem &ps);

    VirtualCharacterController(const VirtualCharacterController &) = delete;
    auto operator=(const VirtualCharacterController &) -> VirtualCharacterController & = delete;
    VirtualCharacterController(VirtualCharacterController &&) = delete;
    auto operator=(VirtualCharacterController &&) -> VirtualCharacterController & = delete;

    auto update(std::chrono::milliseconds delta) -> void;

    auto debug_draw(PhysicsDebugRenderer &renderer) -> void;

    auto position() const -> Vector3;

    auto rotation() const -> Quaternion;

  private:
    ::JPH::PhysicsSystem &ps_;
    ::JPH::RefConst<::JPH::Shape> shape_;
    ::JPH::Ref<::JPH::CharacterVirtual> character_;
};

}
