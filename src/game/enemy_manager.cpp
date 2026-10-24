#include "game/enemy_manager.h"

#include <ranges>

#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/render_entity_manager.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "game/enemy.h"
#include "graphics/window.h"
#include "physics/physics_system.h"

namespace ufps
{

auto EnemyManager::spawn(
    const Transform &transform,
    float health,
    PlayerActor *player,
    Scene &scene,
    const Window &window) -> EnemyHandle
{
    const auto &[ps, cm, em, rem] = services<PhysicsSystem, CameraManager, EntityManager, RenderEntityManager>();

    const auto enemy_camera_handle = cm.insert(
        {{},
         std::numbers::pi_v<float> / 4.0f,
         static_cast<float>(window.window_width()),
         static_cast<float>(window.window_height()),
         0.1f,
         100.0f});
    const auto enemy_entity = em.insert({"enemy", EntityType::DYNAMIC, rem["cube"], transform});
    em[enemy_entity]->set_camera(enemy_camera_handle);
    em[enemy_entity]->set_ray_intersect_rigid_bodies(
        ps.create_meshes(BroadPhaseLayer::STATIC, ObjectLayer::ENEMIES, enemy_entity));
    scene.add(enemy_entity);

    return enemies_.emplace(enemy_entity, health, ps.create_virtual_character_controller(), player);
}

auto EnemyManager::despawn(EnemyHandle handle, Scene &scene) -> void
{
    const auto enemy = enemies_[handle];
    contract_assert(enemy);

    const auto entity_handle = enemy->entity();
    scene.remove(entity_handle);

    auto &ps = service<PhysicsSystem>();
    ps.remove(enemy->character_controller());

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
