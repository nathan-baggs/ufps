#include "game/enemy.h"

#include <chrono>

#include "core/clock.h"
#include "core/entity.h"
#include "core/entity_manager.h"
#include "utils/log.h"

using namespace std::literals;

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
    if (target_position_)
    {
        const auto time_elapsed = Clock::now() - target_position_->start;

        const auto time_elapsed_s = std::chrono::duration_cast<std::chrono::duration<float>>(time_elapsed).count();
        const auto target_duration_s =
            std::chrono::duration_cast<std::chrono::duration<float>>(target_position_->duration).count();
        const auto lerp_amount = time_elapsed_s / target_duration_s;

        log::debug("lerp amount: {}", lerp_amount);

        auto transform = this->transform();

        if (lerp_amount >= 0.0f)
        {
            transform.position.lerp(lerp_amount, target_position_->position);
        }
        else
        {
            transform.position = target_position_->position;
            target_position_.reset();
        }

        set_transform(transform);
    }
}

auto Enemy::hit(const Ray &bullet, const IntersectionResult &) -> void
{
    const auto transform = this->transform();

    auto new_pos = transform.position + bullet.direction * 2.0f;
    new_pos.y = transform.position.y;

    target_position_ = {
        .start = Clock::now(),
        .duration = 500ms,
        .position = new_pos,
    };
}

Enemy::operator bool() const
{
    return health_ != 0.0f;
}

}
