#include "core/player_actor.h"

#include <numbers>

#include "audio/audio_manager.h"
#include "core/actor.h"
#include "core/camera.h"
#include "core/clock.h"
#include "core/entity_manager.h"
#include "core/gun.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "graphics/colour.h"
#include "graphics/particle_manager.h"
#include "maths/quaternion.h"
#include "maths/random.h"
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
    EntityHandle gun_handle,
    Gun gun,
    const InputMap &input_map,
    VirtualCharacterController &character_controller,
    Scene &scene)
    : Actor{entity}
    , gun_handle_{gun_handle}
    , gun_{std::move(gun)}
    , input_map_{input_map}
    , character_controller_{character_controller}
    , scene_{scene}
    , walk_sound_timer_{}
    , yaw_{std::numbers::pi_v<float>}
    , pitch_{}
{
    service<CameraHandle>() = camera_;
}

auto PlayerActor::gun() const -> const Gun &
{
    return gun_;
}

auto PlayerActor::set_gun(Gun gun) -> void
{
    gun_ = std::move(gun);
}

auto PlayerActor::update(Duration delta) -> void
{
    const auto &[em, cm, am, pm, ps] =
        services<EntityManager, CameraManager, AudioManager, ParticleManager, PhysicsSystem>();

    walk_sound_timer_ += delta;

    if (input_map_.is_any_set<Key::W, Key::A, Key::S, Key::D>())
    {
        if (walk_sound_timer_ >= 600ms)
        {
            am.play("LowMetal_Mono_01.wav");
            walk_sound_timer_ = {};
        }
    }

    const auto camera = cm[camera_];
    contract_assert(camera);

    auto gun_entity = em[gun_handle_];
    contract_assert(gun_entity);

    const auto &[mouse_delta, final_recoil] = gun_.update_movement(delta, *gun_entity, input_map_);

    pitch_ += mouse_delta;
    yaw_ -= input_map_.delta_x;

    character_controller_.set_walk_direction(walk_direction(input_map_, *camera));

    auto player = em[entity_];
    contract_assert(player);

    const auto &transform = player->transform();

    auto new_transform =
        Transform{character_controller_.position(), {1.0f}, Quaternion(yaw_, pitch_ - final_recoil, 0.0f)};
    new_transform.position.y = transform.position.y;

    player->set_transform(new_transform);

    bullets_fired_ = gun_.update_bullets(delta, input_map_, *camera);
}

auto PlayerActor::yield_bullets_fired() -> std::vector<Ray>
{
    auto tmp = std::vector<Ray>{};
    std::ranges::swap(tmp, bullets_fired_);
    return tmp;
}

}
