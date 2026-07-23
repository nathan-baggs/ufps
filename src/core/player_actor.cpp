#include "core/player_actor.h"

#include "core/actor.h"
#include "core/camera.h"
#include "events/input_map.h"
#include "maths/vector3.h"

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

PlayerActor::PlayerActor(Camera camera, const InputMap &input_map, VirtualCharacterController &character_controller)
    : Actor{std::move(camera)}
    , input_map_{input_map}
    , character_controller_{character_controller}
{
}

auto PlayerActor::update() -> void
{
    if (input_map_.delta_x != 0.0f)
    {
        camera_.adjust_yaw(input_map_.delta_x);
    }

    if (input_map_.delta_y != 0.0f)
    {
        camera_.adjust_pitch(-input_map_.delta_y);
    }

    character_controller_.set_walk_direction(walk_direction(input_map_, camera_));

    camera_.set_position(character_controller_.position() + Vector3{0.0f, 2.0f, 0.0f});
}
}
