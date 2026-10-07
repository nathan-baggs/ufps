#include "game/enemy_manager.h"

#include <ranges>

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

auto EnemyManager::operator[](EnemyHandle handle) -> std::optional<Enemy &>
{
    return enemies_[handle];
}

auto EnemyManager::operator[](EntityHandle handle) -> std::optional<Enemy &>
{
    auto enemy = std::ranges::find_if(enemies_.data(), [handle](auto &e) { return e.entity() == handle; });
    return enemy == std::ranges::cend(enemies_.data()) ? std::nullopt : std::optional<Enemy &>(*enemy);
}
}
