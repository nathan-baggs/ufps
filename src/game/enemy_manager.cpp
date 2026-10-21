#include "game/enemy_manager.h"

#include <ranges>

#include "core/clock.h"
#include "core/service_locator.h"
#include "game/enemy.h"
#include "physics/physics_system.h"

namespace ufps
{
auto EnemyManager::despawn(EnemyHandle handle) -> void
{
    const auto enemy = enemies_[handle];
    contract_assert(enemy);

    auto &ps = service<PhysicsSystem>();
    ps.remove_virtual_character_controller(enemy->character_controller());

    enemies_.remove(handle);
}

auto EnemyManager::yield_dead_enemies() -> std::vector<EnemyHandle>
{
    auto tmp = std::vector<EnemyHandle>{};
    std::ranges::swap(tmp, dead_enemies_);

    return tmp;
}

auto EnemyManager::update(Duration delta) -> void
{
    for (const auto enemy_handle : enemies_.handles())
    {
        if (const auto enemy = enemies_[enemy_handle]; enemy)
        {
            enemy->update(delta);

            if (enemy->health() <= 0.0f)
            {
                dead_enemies_.push_back(enemy_handle);
            }
        }
    }
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
