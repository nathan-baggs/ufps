#pragma once

#include <optional>

#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"
#include "game/enemy.h"
#include "maths/transform.h"

namespace ufps
{

class PlayerActor;
class Scene;
class Window;

class EnemyManager
{
  public:
    auto spawn(const Transform &transform, float health, PlayerActor *player, Scene &scene, const Window &window)
        -> EnemyHandle;

    auto despawn(EnemyHandle handle, Scene &scene) -> void;

    auto yield_dead_enemies() -> std::vector<EnemyHandle>;

    auto update(Duration delta) -> void;

    auto operator[](EnemyHandle handle) -> std::optional<Enemy &>;

    auto operator[](EntityHandle handle) -> std::optional<Enemy &>;

  private:
    SparseSet<Enemy> enemies_;
    std::vector<EnemyHandle> dead_enemies_;
};

}
