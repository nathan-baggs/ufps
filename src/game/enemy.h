#pragma once

#include "core/actor.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"

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
    explicit operator bool() const;

  private:
    float health_;
};

using EnemyHandle = SparseSet<Enemy>::handle_type;

}
