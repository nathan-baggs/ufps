#pragma once

#include <optional>

#include "core/actor.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"
#include "maths/vector3.h"
#include "physics/physics_system.h"

namespace ufps
{

class Enemy : public Actor
{
  public:
    Enemy(EntityHandle entity, float initial_health) //
        pre(initial_health > 0.0f);

    auto health() const -> float;
    auto change_health(float delta) -> float;
    auto update(Duration delta) -> void;
    auto hit(const Ray &bullet, const IntersectionResult &intersection) -> void;
    explicit operator bool() const;

  private:
    struct LerpPosition
    {
        TimePoint start;
        Duration duration;
        Vector3 position;
    };

    float health_;
    std::optional<LerpPosition> target_position_;
};

using EnemyHandle = SparseSet<Enemy>::handle_type;

}
