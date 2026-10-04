#include "game/enemy.h"

#include "core/clock.h"
#include "core/entity.h"
#include "core/entity_manager.h"

namespace ufps
{

Enemy::Enemy(EntityHandle entity, float initial_health)
    : Actor{entity}
    , health_{initial_health}
{
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

auto Enemy::update(Duration) -> void
{
}

Enemy::operator bool() const
{
    return health_ != 0.0f;
}

}
