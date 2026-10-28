#pragma once

#include <optional>
#include <unordered_map>

#include "core/entity.h"
#include "core/sparse_set.h"
#include "maths/aabb.h"
#include "maths/ray.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_debug_renderer.h"
#include "physics/physics_layers.h"
#include "physics/rigid_body.h"
#include "physics/utils.h"
#include "physics/virtual_character_controller.h"

namespace ufps
{

struct IntersectionResult
{
    EntityHandle entity;
    RigidBodyHandle body;
    Vector3 position;
    Vector3 normal;
    float distance;
};

enum class DebugRenderMode
{
    ON,
    OFF
};

class PhysicsSystem : public ::JPH::ContactListener
{
  public:
    PhysicsSystem(DebugRenderMode debug_render_mode = DebugRenderMode::OFF);
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    auto operator=(const PhysicsSystem &) -> PhysicsSystem & = delete;
    PhysicsSystem(PhysicsSystem &&) = delete;
    auto operator=(PhysicsSystem &&) -> PhysicsSystem & = delete;

    auto create_box(
        const AABB &aabb,
        const Vector3 &position,
        BroadPhaseLayer broad_phase_layer,
        ObjectLayer object_layer,
        EntityHandle entity) -> RigidBodyHandle;

    auto create_meshes(BroadPhaseLayer broad_phase_layer, ObjectLayer object_layer, EntityHandle entity_handle)
        -> std::vector<RigidBodyHandle>;

    auto create_rigid_body(const RigidBody::Description &description, EntityHandle entity) -> RigidBodyHandle;

    auto remove(RigidBodyHandle handle) -> void;

    auto remove(std::span<RigidBodyHandle> handles) -> void;

    auto create_virtual_character_controller() -> VirtualCharacterControllerHandle;

    auto remove(VirtualCharacterControllerHandle handle) -> void;

    auto duplicate(RigidBodyHandle handle) -> RigidBodyHandle;

    auto operator[](this auto &&self, RigidBodyHandle);

    auto operator[](this auto &&self, VirtualCharacterControllerHandle);

    auto update() -> void;

    auto draw_debug(const Vector3 &position) -> void;

    auto cast_ray(const Ray &ray) const -> std::optional<IntersectionResult>;

    auto debug_renderer() -> std::optional<PhysicsDebugRenderer &>;

    auto gravity() const -> Vector3;

    auto set_gravity(const Vector3 &gravity) -> void;

  private:
    SimpleBroadPhaseLayer broad_phase_layer_;
    SimpleObjectVsBroadPhaseLayerFilter object_vs_broad_phase_layer_filter_;
    SimpleObjectLayerPairFilter object_layer_pair_filter_;
    CastRayObjectLayerFilter cast_ray_layer_filter_;
    IgnoreLayerDrawFilter ignore_layer_draw_filter_;
    ::JPH::TempAllocatorImpl temp_allocator_;
    ::JPH::JobSystemThreadPool job_system_;
    ::JPH::PhysicsSystem physics_system_;
    SparseSet<RigidBody> rigid_bodies_;
    SparseSet<VirtualCharacterController> character_controllers_;
    std::optional<PhysicsDebugRenderer> debug_renderer_;
    std::unordered_map<MeshView, ::JPH::Ref<::JPH::Shape>> mesh_shape_cache_;
};

auto PhysicsSystem::operator[](this auto &&self, RigidBodyHandle handle)
{
    return self.rigid_bodies_[handle];
}

auto PhysicsSystem::operator[](this auto &&self, VirtualCharacterControllerHandle handle)
{
    return self.character_controllers_[handle];
}

}
