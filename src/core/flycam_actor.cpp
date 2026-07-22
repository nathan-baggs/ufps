#include "core/flycam_actor.h"

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

    if (input_map[ufps::Key::Q])
    {
        direction += camera.up();
    }

    if (input_map[ufps::Key::E])
    {
        direction -= camera.up();
    }

    return ufps::Vector3::normalise(direction);
}
}

namespace ufps
{

FlyCamActor::FlyCamActor(Camera camera, const InputMap &input_map)
    : Actor{std::move(camera)}
    , input_map_{input_map}
{
}

auto FlyCamActor::update() -> void
{
    if (input_map_.delta_x != 0.0f)
    {
        camera_.adjust_yaw(input_map_.delta_x);
    }

    if (input_map_.delta_y != 0.0f)
    {
        camera_.adjust_pitch(-input_map_.delta_y);
    }

    static const auto speed = 0.1f;
    camera_.translate(walk_direction(input_map_, camera_) * speed);
}
}
