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
};

class ParticleManager
{
  public:
    ParticleManager();

    auto spawn_sparks(const Vector3 &position) -> void;

    auto particles() const -> std::span<const Particle>;

  private:
    std::vector<Particle> particles_;
    std::vector<Particle>::iterator next_;
};

}
