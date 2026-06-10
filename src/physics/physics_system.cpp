#include "physics/physics_system.h"

#include <cstdarg>
#include <cstdio>
#include <string_view>

#include "Jolt/Core/Memory.h"
#include "physics/jolt.h"
#include "physics/utils.h"
#include "utils/error.h"
#include "utils/log.h"

namespace
{

auto jolt_trace(const char *fmt, ...) -> void
{
    auto list = ::va_list{};
    ::va_start(list, fmt);

    auto buffer = std::array<char, 1024zu>{};
    const auto write_count = ::vsnprintf(buffer.data(), sizeof(buffer), fmt, list);
    ::va_end(list);

    ufps::ensure(write_count > 0, "failed to jolt trace");

    ufps::log::info("jolt_trace: {}", std::string_view(buffer.data(), write_count));
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

auto PhysicsSystem::update() -> void
{
    physics_system_.Update(1.0f / 60.f, 1, &temp_allocator_, &job_system_);
}

}
