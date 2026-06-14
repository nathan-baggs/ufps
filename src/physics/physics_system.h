#pragma once

#include "core/sparse_set.h"
#include "maths/aabb.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_layers.h"
#include "physics/rigid_body.h"
#include "physics/utils.h"

namespace ufps
{

using RigidBodyHandle = SparseSet<RigidBody>::handle_type;

class PhysicsSystem
{
  public:
    PhysicsSystem();
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    auto operator=(const PhysicsSystem &) -> PhysicsSystem & = delete;
    PhysicsSystem(PhysicsSystem &&) = delete;
    auto operator=(PhysicsSystem &&) -> PhysicsSystem & = delete;

    auto create_box(const AABB &aabb, const Vector3 &position, PhysicsLayer layer) -> RigidBodyHandle;

    constexpr auto rigid_body(this auto &&self, RigidBodyHandle handle);

    auto update() -> void;

  private:
    SimpleBroadPhaseLayer broad_phase_layer_;
    SimpleObjectVsBroadPhaseLayerFilter object_vs_broad_phase_layer_filter_;
    SimpleObjectLayerPairFilter object_layer_pair_filter_;
    ::JPH::TempAllocatorImpl temp_allocator_;
    ::JPH::JobSystemThreadPool job_system_;
    ::JPH::PhysicsSystem physics_system_;
    SparseSet<RigidBody> rigid_bodies_;
};

constexpr auto PhysicsSystem::rigid_body(this auto &&self, RigidBodyHandle handle)
{
    return self.rigid_bodies_[handle];
}

}
