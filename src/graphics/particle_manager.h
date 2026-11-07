#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "maths/cone.h"

namespace ufps
{

struct alignas(16) Particle
{
    Vector3 position;
    float life;
    Vector3 velocity;
    float pad = {};
};

class ParticleManager
{
  public:
    ParticleManager();

    auto spawn_sparks(const Cone &cone, std::uint32_t count) -> void;

    template <class Self>
    auto particles(this Self &&self)
    {
        using SpanType = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Particle, Particle>;
        return std::span<SpanType>(self.particles_);
    }

  private:
    std::vector<Particle> particles_;
    std::vector<Particle>::iterator next_;
};

}
