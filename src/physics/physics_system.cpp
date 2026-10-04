#include "physics/physics_system.h"

#include <contracts>
#include <cstdarg>
#include <cstdio>
#include <optional>
#include <ranges>
#include <string_view>

#include "Jolt/Math/Float3.h"
#include "Jolt/Physics/Body/BodyLock.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "core/entity.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "graphics/mesh_manager.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_layers.h"
#include "physics/utils.h"
#include "physics/virtual_character_controller.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/formatter.h"
#include "utils/log.h"

using namespace std::literals;

namespace
{

struct HandlePair
{
    ufps::EntityHandle entity;
    ufps::RigidBodyHandle body;
};

auto to_activation(ufps::BroadPhaseLayer layer) -> ::JPH::EActivation
{
    switch (layer)
    {
        using enum ufps::BroadPhaseLayer;
        case STATIC: return ::JPH::EActivation::DontActivate;
        case DYNAMIC: return ::JPH::EActivation::Activate;
    }

    throw ufps::Exception("unknown layer type: {}", layer);
}

auto to_motion(ufps::BroadPhaseLayer layer) -> ::JPH::EMotionType
{
    switch (layer)
    {
        using enum ufps::BroadPhaseLayer;
        case STATIC: return ::JPH::EMotionType::Static;
        case DYNAMIC: return ::JPH::EMotionType::Dynamic;
    }

    throw ufps::Exception("unknown layer type: {}", layer);
}

auto jolt_trace(const char *fmt, ...) -> void
{
    auto list = ::va_list{};
    ::va_start(list, fmt);

    auto buffer = std::array<char, 1024zu>{};
    const auto write_count = ::vsnprintf(buffer.data(), sizeof(buffer), fmt, list);
    ::va_end(list);

    ufps::ensure(write_count > 0, "failed to jolt trace");

    const auto error_str = std::string_view(buffer.data(), write_count);
    if (error_str.starts_with("Error"))
    {
        throw ufps::Exception{"{}", error_str};
    }

    ufps::log::info("jolt_trace: {}", error_str);
}

auto jolt_init = []
{
    ::JPH::RegisterDefaultAllocator();
    ::JPH::Trace = jolt_trace;
    ::JPH::Factory::sInstance = new ::JPH::Factory{};
    ::JPH::RegisterTypes();
    return true;
}();

}

namespace ufps
{

PhysicsSystem::PhysicsSystem(DebugRenderMode debug_render_mode)
    : broad_phase_layer_{}
    , object_vs_broad_phase_layer_filter_{}
    , object_layer_pair_filter_{}
    , cast_ray_layer_filter_{}
    , ignore_layer_draw_filter_{ObjectLayer::LEVEL_GEOMETRY}
    , temp_allocator_{10u * 1024u * 1024u}
    , job_system_{::JPH::cMaxPhysicsJobs, ::JPH::cMaxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() - 1zu)}
    , physics_system_{}
    , debug_renderer_{debug_render_mode == DebugRenderMode::ON ? std::make_optional<PhysicsDebugRenderer>() : std::nullopt}
    , player_controller_{}
    , mesh_shape_cache_{}
{
    constexpr auto max_bodies = 2048u;
    constexpr auto num_body_mutexes = 0u;
    constexpr auto max_body_pairs = 1024u;
    constexpr auto max_contact_constraints = 1024u;

    physics_system_.Init(
        max_bodies,
        num_body_mutexes,
        max_body_pairs,
        max_contact_constraints,
        broad_phase_layer_,
        object_vs_broad_phase_layer_filter_,
        object_layer_pair_filter_);

    physics_system_.SetGravity({0.0f, -9.8f, 0.0f});
    physics_system_.SetContactListener(this);

    player_controller_ = std::make_unique<VirtualCharacterController>(physics_system_);
}

auto PhysicsSystem::create_box(
    const AABB &aabb,
    const Vector3 &position,
    BroadPhaseLayer broad_phase_layer,
    ObjectLayer object_layer,
    EntityHandle entity) -> RigidBodyHandle
{
    const auto half_extents =
        Vector3{(aabb.max.x - aabb.min.x) / 2.0f, (aabb.max.y - aabb.min.y) / 2.0f, (aabb.max.z - aabb.min.z) / 2.0f};

    auto box_shape_settings = ::JPH::BoxShapeSettings{to_jolt(half_extents)};
    box_shape_settings.SetEmbedded();

    auto box_result = box_shape_settings.Create();
    if (box_result.HasError())
    {
        throw Exception("box error: {}", box_result.GetError());
    }

    const auto &box = box_result.Get();

    const auto body_settings = ::JPH::BodyCreationSettings{
        box,
        to_jolt(position),
        ::JPH::Quat::sIdentity(),
        to_motion(broad_phase_layer),
        static_cast<::JPH::ObjectLayer>(object_layer)};
    auto &interface = physics_system_.GetBodyInterface();

    const auto body_id = interface.CreateAndAddBody(body_settings, to_activation(broad_phase_layer));

    auto rb = rigid_bodies_.emplace(body_id, std::addressof(interface), broad_phase_layer, object_layer);

    static_assert(sizeof(HandlePair) == sizeof(std::uint64_t));

    const auto handle_pair = HandlePair{.entity = entity, .body = rb};
    const auto user_data = std::bit_cast<std::uint64_t>(handle_pair);

    interface.SetUserData(body_id, user_data);

    return rb;
}

auto PhysicsSystem::create_meshes(
    BroadPhaseLayer broad_phase_layer,
    ObjectLayer object_layer,
    EntityHandle entity_handle) -> std::vector<RigidBodyHandle>
{
    auto handles = std::vector<RigidBodyHandle>();

    const auto &[em, rem, mm] = services<EntityManager, RenderEntityManager, MeshManager>();

    const auto &entity = em[entity_handle];
    contract_assert(entity);

    for (const auto render_entity_handle : entity->render_entities())
    {
        const auto &render_entity = rem[render_entity_handle];
        contract_assert(render_entity);

        const auto mesh_view = render_entity->mesh_view();

        auto shape = ::JPH::Ref<::JPH::Shape>{};

        const auto find_shape = mesh_shape_cache_.find(mesh_view);
        if (find_shape == std::ranges::cend(mesh_shape_cache_))
        {
            const auto jolt_vertex_list =
                mm.vertex_data(mesh_view) |
                std::views::transform([](const auto &e) { return std::bit_cast<::JPH::Float3>(e.position); }) |
                std::ranges::to<::JPH::Array<::JPH::Float3>>();

            const auto jolt_index_list =
                mm.index_data(mesh_view) | std::views::chunk(3u) |
                std::views::transform([](const auto &e) { return ::JPH::IndexedTriangle{e[0], e[1], e[2]}; }) |
                std::ranges::to<::JPH::Array<::JPH::IndexedTriangle>>();

            auto mesh_shape_settings = ::JPH::MeshShapeSettings{jolt_vertex_list, jolt_index_list};
            mesh_shape_settings.SetEmbedded();

            auto mesh_result = mesh_shape_settings.Create();
            if (mesh_result.HasError())
            {
                throw Exception("mesh error: {}", mesh_result.GetError());
            }

            const auto &mesh_shape = mesh_result.Get();
            mesh_shape_cache_.insert({mesh_view, mesh_shape});
            shape = mesh_shape;
        }
        else
        {
            shape = find_shape->second;
        }

        const auto body_settings = ::JPH::BodyCreationSettings{
            shape,
            to_jolt(entity->transform().position),
            ::JPH::Quat::sIdentity(),
            to_motion(broad_phase_layer),
            static_cast<::JPH::ObjectLayer>(object_layer)};
        auto &interface = physics_system_.GetBodyInterface();

        const auto body_id = interface.CreateAndAddBody(body_settings, to_activation(broad_phase_layer));

        auto rb = rigid_bodies_.emplace(body_id, std::addressof(interface), broad_phase_layer, object_layer);

        static_assert(sizeof(HandlePair) == sizeof(std::uint64_t));

        const auto handle_pair = HandlePair{.entity = entity_handle, .body = rb};
        const auto user_data = std::bit_cast<std::uint64_t>(handle_pair);

        interface.SetUserData(body_id, user_data);

        handles.push_back(rb);
    }

    return handles;
}

auto PhysicsSystem::create_rigid_body(const RigidBody::Description &description, EntityHandle entity) -> RigidBodyHandle
{
    const auto transform = Transform{description.local_transform};
    const auto handle = create_box(
        {{-1.0f}, {1.0f}}, transform.position, description.broad_phase_layer, description.object_layer, entity);

    rigid_bodies_[handle]->set_local_transform(transform);

    return handle;
}

auto PhysicsSystem::remove_rigid_body(RigidBodyHandle handle) -> void
{
    const auto &rb = rigid_body(handle);
    contract_assert(rb);

    physics_system_.GetBodyInterface().RemoveBody(rb->native_handle());

    rigid_bodies_.remove(handle);
}

auto PhysicsSystem::duplicate_rigid_body(RigidBodyHandle handle) -> RigidBodyHandle
{
    const auto &rb = rigid_body(handle);
    contract_assert(rb);

    const auto handle_pair = std::bit_cast<HandlePair>(rb->user_data());
    contract_assert(handle_pair.body == handle);

    return create_rigid_body(rb->description(), handle_pair.entity);
}

auto PhysicsSystem::update() -> void
{
    player_controller_->update(16ms, to_native(physics_system_.GetGravity()));
    physics_system_.Update(1.0f / 60.f, 1, &temp_allocator_, &job_system_);

    if (debug_renderer_)
    {
        static const auto settings = ::JPH::BodyManager::DrawSettings{};
        physics_system_.DrawBodies(
            settings, std::addressof(*debug_renderer_), std::addressof(ignore_layer_draw_filter_));
        player_controller_->debug_draw(*debug_renderer_);
    }
}

auto PhysicsSystem::cast_ray(const Ray &ray) const -> std::optional<IntersectionResult>
{
    const auto jolt_ray = to_jolt(ray);
    auto hit_result = ::JPH::RayCastResult{};

    const auto hit = physics_system_.GetNarrowPhaseQuery().CastRay(jolt_ray, hit_result, {}, cast_ray_layer_filter_);

    if (hit)
    {
        auto &interface = physics_system_.GetBodyInterface();
        const auto body_id = hit_result.mBodyID;
        const auto handle_pair = std::bit_cast<HandlePair>(interface.GetUserData(body_id));

        if (const auto lock = ::JPH::BodyLockRead{physics_system_.GetBodyLockInterface(), body_id}; lock.Succeeded())
        {
            const auto hit_pos = jolt_ray.GetPointOnRay(hit_result.mFraction);

            const auto &body = lock.GetBody();
            const auto normal = body.GetWorldSpaceSurfaceNormal(hit_result.mSubShapeID2, hit_pos);

            return IntersectionResult{
                .entity = handle_pair.entity,
                .body = handle_pair.body,
                .position = to_native(hit_pos),
                .normal = to_native(normal),
                .distance = (hit_pos - jolt_ray.mOrigin).Length(),
            };
        }
    }

    return {};
}

auto PhysicsSystem::debug_renderer() -> std::optional<PhysicsDebugRenderer &>
{
    return debug_renderer_.transform([](auto &e) -> decltype(auto) { return e; });
}

auto PhysicsSystem::player_controller() -> VirtualCharacterController &
{
    return *player_controller_;
}
}
