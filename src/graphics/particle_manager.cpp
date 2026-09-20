#include "graphics/particle_manager.h"
#include "maths/vector3.h"

#include <ranges>
#include <span>
#include <vector>

namespace ufps
{

ParticleManager::ParticleManager()
    : particles_(100zu, {.position = {}, .life = 0.0f, .velocity = {}})
    , next_{std::ranges::begin(particles_)}
{
}

auto ParticleManager::spawn_sparks(const Vector3 &position, const Vector3 &velocity) -> void
{
    *next_ = {
        .position = position,
        .life = 1.0f,
        .velocity = velocity,
        .pad = 1.0f,
    };

    ++next_;
    if (next_ == std::ranges::end(particles_))
    {
        next_ = std::ranges::begin(particles_);
    }
}

auto ParticleManager::particles() const -> std::span<const Particle>
{
    return particles_;
}

auto ParticleManager::particles() -> std::span<Particle>
{
    return particles_;
}
}
