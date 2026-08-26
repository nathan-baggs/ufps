#include "core/player_actor.h"

#include "audio/audio_manager.h"
#include "core/actor.h"
#include "core/camera.h"
#include "core/entity_manager.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "graphics/colour.h"
#include "graphics/debug_layer.h"
#include "maths/quaternion.h"
#include "maths/ray.h"
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
    VirtualCharacterController &character_controller,
    const Scene &scene)
    : Actor{entity}
    , input_map_{input_map}
    , character_controller_{character_controller}
    , scene_{scene}
{
    service<CameraHandle>() = camera_;
}

auto PlayerActor::update() -> void
{
    const auto &[em, cm, dl, am] = services<EntityManager, CameraManager, DebugLayer, AudioManager>();

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

    static auto handle_click = true;

    if (const auto mouse_event = input_map_.mouse_event; mouse_event)
    {
        if (handle_click && mouse_event->state() == MouseButtonState::DOWN)
        {
            am.play("Specter Bullet.wav");

            const auto bullet_ray = Ray{camera->transform().position, camera->direction() * 100.0f};

            if (const auto intersection = scene_.intersect_ray(bullet_ray); intersection)
            {
                pew_pew_lines_.push_back(
                    std::make_tuple(
                        intersection->position, intersection->position + (intersection->normal * 0.5f), colours::blue));

                pew_pew_lines_.push_back(
                    std::make_tuple(
                        bullet_ray.origin,
                        bullet_ray.origin + (bullet_ray.direction * intersection->distance),
                        colours::hot_pink));
            }
            else
            {
                pew_pew_lines_.push_back(
                    std::make_tuple(
                        bullet_ray.origin, bullet_ray.origin + (bullet_ray.direction * 100.0f), colours::red));
            }

            handle_click = false;
        }
    }
    else
    {
        handle_click = true;
    }

    for (const auto &[start, end, colour] : pew_pew_lines_)
    {
        dl.push_line(start, end, colour, DebugLayerType::DEFAULT);
    }
}
}
