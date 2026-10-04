#include "game/enemy_manager.h"

#include "core/clock.h"
#include "game/enemy.h"

namespace ufps
{
auto EnemyManager::despawn(EnemyHandle handle) -> void
{
    enemies_.remove(handle);
}

auto EnemyManager::update(Duration) -> void
{
}
}
