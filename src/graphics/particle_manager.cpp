#include "graphics/particle_manager.h"

#include <ranges>
#include <span>
#include <vector>

#include "maths/cone.h"
#include "maths/random.h"
#include "maths/vector3.h"

namespace ufps
{

ParticleManager::ParticleManager()
    : particles_(100zu, {.position = {}, .life = 0.0f, .velocity = {}})
    , next_{std::ranges::begin(particles_)}
{
}

auto ParticleManager::spawn_sparks(const Cone &cone, std::uint32_t count) -> void
{
    const auto position = cone.origin;

    for (auto i = 0u; i < count; ++i)
    {
        *next_ = {
            .position = position,
            .life = 3.0f,
            .velocity = random::rand_vector3(cone),
            .pad = 1.0f,
        };

        ++next_;
        if (next_ == std::ranges::end(particles_))
        {
            next_ = std::ranges::begin(particles_);
        }
    }
}
}
