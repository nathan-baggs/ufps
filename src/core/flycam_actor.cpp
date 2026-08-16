#include "core/flycam_actor.h"

#include "core/actor.h"
#include "core/camera.h"
#include "core/entity_manager.h"
#include "events/input_map.h"
#include "maths/vector3.h"

namespace
{
[[maybe_unused]] auto walk_direction(const ufps::InputMap &input_map, const ufps::Camera &camera) -> ufps::Vector3
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

FlyCamActor::FlyCamActor(CameraHandle camera, EntityHandle entity, const InputMap &input_map)
    : Actor{camera}
    , input_map_{input_map}
    , entity_{entity}
{
}

auto FlyCamActor::update() -> void
{
    const auto &[em, cm] = services<EntityManager, CameraManager>();

    const auto camera = cm[camera_];
    contract_assert(camera);

    static auto yaw = 0.0f;
    static auto pitch = 0.0f;

    yaw += input_map_.delta_y;
    pitch -= input_map_.delta_x;

    auto e = em[entity_];
    contract_assert(e);

    static const auto speed = 0.1f;
    const auto transform = e->transform();
    const auto new_transform = Transform{
        transform.position + (walk_direction(input_map_, *camera) * speed), {1.0f}, Quaternion(yaw, pitch, 0.0f)};

    e->set_transform(new_transform);
}
}
