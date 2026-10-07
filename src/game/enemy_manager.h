#pragma once

#include <optional>

#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"
#include "game/enemy.h"

namespace ufps
{

class EnemyManager
{
  public:
    template <class... Args>
    auto spawn(Args &&...args) -> EnemyHandle
    {
        return enemies_.emplace(std::forward<Args>(args)...);
    }

    auto despawn(EnemyHandle handle) -> void;

    auto update(Duration delta) -> void;

    auto operator[](EnemyHandle handle) -> std::optional<Enemy &>;

    auto operator[](EntityHandle handle) -> std::optional<Enemy &>;

  private:
    SparseSet<Enemy> enemies_;
};

}
