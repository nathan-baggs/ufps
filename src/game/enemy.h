#pragma once

#include <optional>
#include <vector>

#include "core/actor.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/sparse_set.h"
#include "game/nav_mesh.h"
#include "maths/vector3.h"
#include "physics/physics_system.h"
#include "physics/virtual_character_controller.h"

namespace ufps
{

class PlayerActor;

class Enemy : public Actor
{
    enum class State
    {
        ROAMING,
        AGRO,
        FLEE,
    };

  public:
    Enemy(
        EntityHandle entity,
        float initial_health,
        VirtualCharacterControllerHandle character_controller_handle,
        PlayerActor *player) //
        pre(initial_health > 0.0f);

    auto health() const -> float;
    auto change_health(float delta) -> float;
    auto update(Duration delta) -> void;
    auto hit(const Ray &bullet, const IntersectionResult &intersection) -> void;
    explicit operator bool() const;
    auto character_controller() const -> VirtualCharacterControllerHandle;

  private:
    float max_health_;
    float health_;
    VirtualCharacterControllerHandle character_controller_handle_;
    PlayerActor *player_;
    State state_;
    std::vector<NavMesh::Node> route_;
    std::span<NavMesh::Node> current_route_;
};

using EnemyHandle = SparseSet<Enemy>::handle_type;

}
