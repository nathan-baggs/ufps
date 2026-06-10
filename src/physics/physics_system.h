#pragma once

#include "physics/jolt.h"
#include "physics/utils.h"

namespace ufps
{

class PhysicsSystem
{
  public:
    PhysicsSystem();
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    auto operator=(const PhysicsSystem &) -> PhysicsSystem & = delete;
    PhysicsSystem(PhysicsSystem &&) = delete;
    auto operator=(PhysicsSystem &&) -> PhysicsSystem & = delete;

    auto update() -> void;

  private:
    SimpleBroadPhaseLayer broad_phase_layer_;
    SimpleObjectVsBroadPhaseLayerFilter object_vs_broad_phase_layer_filter_;
    SimpleObjectLayerPairFilter object_layer_pair_filter_;
    ::JPH::TempAllocatorImpl temp_allocator_;
    ::JPH::JobSystemThreadPool job_system_;
    ::JPH::PhysicsSystem physics_system_;
};

}
