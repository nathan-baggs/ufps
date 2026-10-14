#pragma once

#include <optional>

#include "core/actor.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"
#include "maths/vector3.h"
#include "physics/physics_system.h"
#include "physics/virtual_character_controller.h"

namespace ufps
{

class Enemy : public Actor
{
  public:
    Enemy(EntityHandle entity, float initial_health, VirtualCharacterControllerHandle character_controller_handle) //
        pre(initial_health > 0.0f);

    auto health() const -> float;
    auto change_health(float delta) -> float;
    auto update(Duration delta) -> void;
    auto hit(const Ray &bullet, const IntersectionResult &intersection) -> void;
    explicit operator bool() const;

  private:
    struct LerpPosition
    {
        TimePoint start;
        Duration duration;
        Vector3 position;
    };

    float max_health_;
    float health_;
    std::optional<LerpPosition> target_position_;
    VirtualCharacterControllerHandle character_controller_handle_;
};

using EnemyHandle = SparseSet<Enemy>::handle_type;

}
