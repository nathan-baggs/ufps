#include "game/enemy.h"

#include <chrono>

#include "core/clock.h"
#include "core/entity.h"
#include "core/entity_manager.h"
#include "core/player_actor.h"
#include "maths/vector3.h"
#include "physics/virtual_character_controller.h"
#include "utils/log.h"

using namespace std::literals;

namespace ufps
{

Enemy::Enemy(
    EntityHandle entity,
    float initial_health,
    VirtualCharacterControllerHandle character_controller_handle,
    PlayerActor *player)
    : Actor{entity}
    , max_health_{initial_health}
    , health_{max_health_}
    , character_controller_handle_{character_controller_handle}
    , player_{player}
    , hit_cooldown_{}
{
    const auto &[ps] = services<PhysicsSystem>();
    const auto controller = ps[character_controller_handle_];
    ensure(controller, "missing character controller");

    controller->set_position(transform().position);
}

auto Enemy::health() const -> float
{
    return health_;
}

auto Enemy::change_health(float delta) -> float
{
    health_ = std::max(0.0f, health_ + delta);
    return health_;
}

auto Enemy::update(Duration delta) -> void
{
    const auto &[ps] = services<PhysicsSystem>();
    auto controller = ps[character_controller_handle_];
    contract_assert(controller);

    auto transform = this->transform();

    auto walk_direction = Vector3{};

    transform.position = controller->position();
    set_transform(transform);

    if (hit_cooldown_ <= 0ms)
    {
        walk_direction += Vector3::normalise(player_->transform().position - transform.position);
    }
    else
    {
        hit_cooldown_ -= delta;
    }

    walk_direction.y = 0.0f;
    controller->set_walk_direction(walk_direction);
}

auto Enemy::hit(const Ray &, const IntersectionResult &) -> void
{
    hit_cooldown_ = 500ms;
    health_ -= 10.0f;
}

Enemy::operator bool() const
{
    return health_ != 0.0f;
}

auto Enemy::character_controller() const -> VirtualCharacterControllerHandle
{
    return character_controller_handle_;
}

}
