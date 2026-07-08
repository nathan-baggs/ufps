#pragma once

#include <optional>

#include "core/sparse_set.h"
#include "maths/aabb.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_debug_renderer.h"
#include "physics/physics_layers.h"
#include "physics/rigid_body.h"
#include "physics/utils.h"

namespace ufps
{

using RigidBodyHandle = SparseSet<RigidBody>::handle_type;

enum class DebugRenderMode
{
    ON,
    OFF
};

class PhysicsSystem
{
  public:
    PhysicsSystem(DebugRenderMode debug_render_mode = DebugRenderMode::OFF);
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    auto operator=(const PhysicsSystem &) -> PhysicsSystem & = delete;
    PhysicsSystem(PhysicsSystem &&) = delete;
    auto operator=(PhysicsSystem &&) -> PhysicsSystem & = delete;

    auto create_box(const AABB &aabb, const Vector3 &position, PhysicsLayer layer) -> RigidBodyHandle;

    auto create_rigid_body(const RigidBody::Description &description) -> RigidBodyHandle;
    auto remove_rigid_body(RigidBodyHandle handle) -> void;

    constexpr auto rigid_body(this auto &&self, RigidBodyHandle handle);

    auto update() -> void;

    auto debug_renderer() -> std::optional<PhysicsDebugRenderer &>;

  private:
    SimpleBroadPhaseLayer broad_phase_layer_;
    SimpleObjectVsBroadPhaseLayerFilter object_vs_broad_phase_layer_filter_;
    SimpleObjectLayerPairFilter object_layer_pair_filter_;
    ::JPH::TempAllocatorImpl temp_allocator_;
    ::JPH::JobSystemThreadPool job_system_;
    ::JPH::PhysicsSystem physics_system_;
    SparseSet<RigidBody> rigid_bodies_;
    std::optional<PhysicsDebugRenderer> debug_renderer_;
};

constexpr auto PhysicsSystem::rigid_body(this auto &&self, RigidBodyHandle handle)
{
    return self.rigid_bodies_[handle];
}

}
