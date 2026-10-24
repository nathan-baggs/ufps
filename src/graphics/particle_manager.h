#pragma once

#include "maths/vector3.h"

#include <span>
#include <vector>

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

    auto spawn_sparks(const Vector3 &position, const Vector3 &velocity) -> void;

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
