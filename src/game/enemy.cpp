#include "game/enemy.h"

#include <chrono>
#include <memory>
#include <vector>

#include "core/clock.h"
#include "core/entity.h"
#include "core/entity_manager.h"
#include "core/player_actor.h"
#include "game/nav_mesh.h"
#include "graphics/debug_layer.h"
#include "maths/random.h"
#include "maths/vector3.h"
#include "physics/virtual_character_controller.h"
#include "utils/log.h"

using namespace std::literals;

namespace
{
static auto g_nav_mesh = std::unique_ptr<ufps::NavMesh>{};

auto random_route(const ufps::Vector3 &start) -> std::vector<ufps::NavMesh::Node>
{
    const auto start_index = g_nav_mesh->node(start);
    contract_assert(start_index);

    const auto end_index = g_nav_mesh->node(ufps::random::rand_element(g_nav_mesh->nodes()).position);
    contract_assert(end_index);

    const auto route = g_nav_mesh->route(*start_index, *end_index);
    contract_assert(route);

    return *route;
}

}

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
    , state_{State::ROAMING}
    , route_{}
    , current_route_{}
{
    [[maybe_unused]] static const auto nav_mesh_init = []
    {
        g_nav_mesh = std::make_unique<NavMesh>();
        return true;
    }();

    route_ = random_route(transform().position);
    current_route_ = route_;

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

auto Enemy::update(Duration) -> void
{
    const auto &[ps, dl] = services<PhysicsSystem, DebugLayer>();
    auto controller = ps[character_controller_handle_];
    contract_assert(controller);

    auto transform = this->transform();

    transform.position = controller->position();
    set_transform(transform);

    if (std::ranges::empty(current_route_))
    {
        route_ = random_route(transform.position);
        current_route_ = route_;
    }

    contract_assert(!std::ranges::empty(current_route_));

    auto walk_direction = Vector3::normalise(current_route_.front().position - transform.position);
    walk_direction.y = 0.0f;
    controller->set_walk_direction(walk_direction);

    dl.push_route(route_, transform.position.y, DebugLayerType::DEFAULT);
    dl.push_route(route_, transform.position.y, DebugLayerType::DEBUG);

    if (Vector3::distance(transform.position, current_route_.front().position) < 0.1f)
    {
        current_route_ = current_route_.subspan(1zu);
    }
}

auto Enemy::hit(const Ray &, const IntersectionResult &) -> void
{
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
