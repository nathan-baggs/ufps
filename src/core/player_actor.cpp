#include "core/player_actor.h"

#include "core/actor.h"
#include "core/camera.h"
#include "events/key_map.h"
#include "maths/vector3.h"

namespace
{
auto walk_direction(const ufps::KeyMap &key_map, const ufps::Camera &camera) -> ufps::Vector3
{
    auto direction = ufps::Vector3{};

    const auto camera_direction = camera.direction();
    const auto forward = ufps::Vector3::normalise({camera_direction.x, 0.0f, camera_direction.z});

    if (key_map[ufps::Key::W])
    {
        direction += forward;
    }

    if (key_map[ufps::Key::S])
    {
        direction -= forward;
    }

    if (key_map[ufps::Key::D])
    {
        direction += camera.right();
    }

    if (key_map[ufps::Key::A])
    {
        direction -= camera.right();
    }

    constexpr auto speed = 0.1f;
    return ufps::Vector3::normalise(direction) * speed;
}
}

namespace ufps
{

PlayerActor::PlayerActor(Camera camera, const KeyMap &key_map)
    : Actor{std::move(camera)}
    , key_map_{key_map}
{
}

auto PlayerActor::update() -> void
{
    if (key_map_.delta_x != 0.0f)
    {
        camera_.adjust_yaw(key_map_.delta_x);
    }

    if (key_map_.delta_y != 0.0f)
    {
        camera_.adjust_pitch(-key_map_.delta_y);
    }

    camera_.translate(walk_direction(key_map_, camera_));
}
}
