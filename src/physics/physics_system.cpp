#include "physics/physics_system.h"

#include <cstdarg>
#include <cstdio>
#include <string_view>

#include "Jolt/Math/Vec3.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "maths/vector3.h"
#include "physics/jolt.h"
#include "physics/physics_layers.h"
#include "physics/utils.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/formatter.h"
#include "utils/log.h"

namespace
{

auto to_activation(ufps::PhysicsLayer layer) -> ::JPH::EActivation
{
    switch (layer)
    {
        using enum ufps::PhysicsLayer;
        case STATIC: return ::JPH::EActivation::DontActivate;
        case DYNAMIC: return ::JPH::EActivation::Activate;
    }

    throw ufps::Exception("unknown layer type: {}", layer);
}

auto to_motion(ufps::PhysicsLayer layer) -> ::JPH::EMotionType
{
    switch (layer)
    {
        using enum ufps::PhysicsLayer;
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

PhysicsSystem::PhysicsSystem()
    : broad_phase_layer_{}
    , object_vs_broad_phase_layer_filter_{}
    , object_layer_pair_filter_{}
    , temp_allocator_{10u * 1024u * 1024u}
    , job_system_{::JPH::cMaxPhysicsJobs, ::JPH::cMaxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() - 1zu)}
    , physics_system_{}
{

    constexpr auto max_bodies = 1024u;
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
}

auto PhysicsSystem::create_box(const AABB &aabb, const Vector3 &position, PhysicsLayer layer) -> RigidBodyHandle
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
        box, to_jolt(position), ::JPH::Quat::sIdentity(), to_motion(layer), static_cast<::JPH::ObjectLayer>(layer)};
    auto &interface = physics_system_.GetBodyInterface();

    const auto body_id = interface.CreateAndAddBody(body_settings, to_activation(layer));

    return rigid_bodies_.emplace(body_id, std::addressof(interface));
}

auto PhysicsSystem::update() -> void
{
    physics_system_.Update(1.0f / 60.f, 1, &temp_allocator_, &job_system_);
}
}
