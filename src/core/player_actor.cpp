#include "core/player_actor.h"

#include "core/actor.h"
#include "core/camera.h"
#include "core/entity_manager.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "maths/quaternion.h"
#include "maths/vector3.h"
#include "utils/error.h"

namespace
{
auto walk_direction(const ufps::InputMap &input_map, const ufps::Camera &camera) -> ufps::Vector3
{
    auto direction = ufps::Vector3{};

    const auto camera_direction = camera.direction();
    const auto forward = ufps::Vector3::normalise({camera_direction.x, 0.0f, camera_direction.z});

    if (input_map[ufps::Key::W])
    {
        direction += forward;
    }

    if (input_map[ufps::Key::S])
    {
        direction -= forward;
    }

    if (input_map[ufps::Key::D])
    {
        direction += camera.right();
    }

    if (input_map[ufps::Key::A])
    {
        direction -= camera.right();
    }

    return ufps::Vector3::normalise(direction);
}
}

namespace ufps
{

PlayerActor::PlayerActor(
    EntityHandle entity,
    const InputMap &input_map,
    VirtualCharacterController &character_controller)
    : Actor{entity}
    , input_map_{input_map}
    , character_controller_{character_controller}
{
}

auto PlayerActor::update() -> void
{
    const auto &[em, cm] = services<EntityManager, CameraManager>();

    const auto camera = cm[camera_];
    contract_assert(camera);

    static auto pitch = 0.0f;
    static auto yaw = std::numbers::pi_v<float>;

    pitch += input_map_.delta_y;
    yaw -= input_map_.delta_x;

    character_controller_.set_walk_direction(walk_direction(input_map_, *camera));

    auto player = em[entity_];
    contract_assert(player);

    const auto &transform = player->transform();

    auto new_transform = Transform{character_controller_.position(), {1.0f}, Quaternion(yaw, pitch, 0.0f)};
    new_transform.position.y = transform.position.y;

    player->set_transform(new_transform);
}
}
