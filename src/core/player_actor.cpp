#include "core/player_actor.h"

#include "audio/audio_manager.h"
#include "core/actor.h"
#include "core/camera.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "graphics/colour.h"
#include "graphics/debug_layer.h"
#include "maths/quaternion.h"
#include "maths/ray.h"
#include "maths/spring.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "utils/error.h"

using namespace std::literals;

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
    EntityHandle gun,
    const InputMap &input_map,
    VirtualCharacterController &character_controller,
    const Scene &scene)
    : Actor{entity}
    , gun_{gun}
    , input_map_{input_map}
    , character_controller_{character_controller}
    , scene_{scene}
    , walk_sound_timer_{}
    , recoil_spring_{0.0f, 0.0f, 0.0f, 2.0f * std::numbers::pi_v<float>}
    , turn_spring_{0.0f, 0.0f, 0.0f, 4.0f * std::numbers::pi_v<float>}
    , bob_spring_{0.0f, 0.0f, 0.0f, 4.0f * std::numbers::pi_v<float>, 0.5f}
    , yaw_{std::numbers::pi_v<float>}
    , pitch_{}
    , recoil_target_{}
{
    service<CameraHandle>() = camera_;
}

auto PlayerActor::update(Duration delta) -> void
{
    const auto &[em, cm, dl, am] = services<EntityManager, CameraManager, DebugLayer, AudioManager>();

    shoot_timer_ += delta;
    walk_sound_timer_ += delta;

    if (input_map_.is_any_set<Key::W, Key::A, Key::S, Key::D>())
    {
        if (walk_sound_timer_ >= 600ms)
        {
            am.play("LowMetal_Mono_01.wav");
            bob_spring_.add_impulse(0.01f);
            walk_sound_timer_ = {};
        }
    }

    const auto camera = cm[camera_];
    contract_assert(camera);

    if (input_map_.mouse_down)
    {
        if (shoot_timer_ >= 100ms)
        {
            shoot_timer_ = {};
            recoil_target_ += 0.02f;

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
        }
    }
    else
    {
        recoil_target_ = {};
    }

    recoil_spring_.set_equilibrium_position(recoil_target_);
    const auto recoil = recoil_spring_.update(delta);

    auto mouse_delta = input_map_.delta_y;

    if (mouse_delta > 0.0f && recoil > 0.0f)
    {
        const auto compensation = std::min(mouse_delta, recoil);
        recoil_spring_.add_impulse(-compensation);
        recoil_target_ = std::max(0.0f, recoil_target_ - compensation);
        recoil_spring_.set_equilibrium_position(recoil_target_);

        mouse_delta -= compensation;
    }

    pitch_ += mouse_delta;
    yaw_ -= input_map_.delta_x;

    character_controller_.set_walk_direction(walk_direction(input_map_, *camera));

    auto player = em[entity_];
    contract_assert(player);

    const auto &transform = player->transform();

    auto new_transform = Transform{character_controller_.position(), {1.0f}, Quaternion(yaw_, pitch_ - recoil, 0.0f)};
    new_transform.position.y = transform.position.y;

    player->set_transform(new_transform);

    auto gun = em[gun_];
    contract_assert(gun);

    const auto gun_pos = turn_spring_.update(delta);

    static auto gun_roll = float{};
    const auto roll_delta = gun_pos - gun_roll;
    gun_roll = gun_pos;

    static auto gun_pitch = float{};
    const auto pitch_delta = recoil - gun_pitch;
    gun_pitch = recoil;

    const auto bob_amount = bob_spring_.update(delta);

    static auto bob = float{};
    const auto bob_delta = bob_amount - bob;
    bob = bob_amount;

    turn_spring_.add_impulse(input_map_.delta_x * 0.05f);
    auto gun_transform = gun->local_transform() * Transform{{}, {1.0f}, Quaternion(0.0f, -pitch_delta, -roll_delta)};
    gun_transform.position.y += bob_delta;
    gun->set_transform(gun_transform);

    for (const auto &[start, end, colour] : pew_pew_lines_)
    {
        dl.push_line(start, end, colour, DebugLayerType::DEFAULT);
    }
}
}
