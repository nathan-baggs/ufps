#include "core/gun.h"

#include "audio/audio_manager.h"
#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "maths/spring.h"

namespace ufps
{

Gun::Gun(Description description)
    : shoot_timer_{}
    , fire_rate_{description.fire_rate}
    , recoil_spring_{0.0f, 0.0f, 0.0f, 2.0f * std::numbers::pi_v<float>}
    , turn_spring_{0.0f, 0.0f, 0.0f, 4.0f * std::numbers::pi_v<float>}
    , bob_spring_{0.0f, 0.0f, 0.0f, 4.0f * std::numbers::pi_v<float>, 0.5f}
    , recoil_target_{}
{
}

auto Gun::update(Duration delta, Entity &entity, const InputMap &input_map, const Camera &camera) -> UpdateResult
{
    auto result = UpdateResult{};

    const auto &[am] = services<AudioManager>();

    shoot_timer_ += delta;

    if (input_map.mouse_down)
    {
        if (shoot_timer_ >= fire_rate_)
        {
            shoot_timer_ = {};
            recoil_target_ += 0.02f;

            am.play("Specter Bullet.wav");

            result.bullets_fired.push_back({camera.transform().position, camera.direction() * 100.0f});
        }
    }
    else
    {
        recoil_target_ = {};
    }

    recoil_spring_.set_equilibrium_position(recoil_target_);

    result.final_recoil = recoil_spring_.update(delta);
    result.final_mouse_y_delta = input_map.delta_y;

    if (result.final_mouse_y_delta > 0.0f && result.final_recoil > 0.0f)
    {
        const auto compensation = std::min(result.final_mouse_y_delta, result.final_recoil);
        recoil_spring_.add_impulse(-compensation);
        recoil_target_ = std::max(0.0f, recoil_target_ - compensation);
        recoil_spring_.set_equilibrium_position(recoil_target_);

        result.final_mouse_y_delta -= compensation;
    }

    const auto gun_pos = turn_spring_.update(delta);

    static auto gun_roll = float{};
    const auto roll_delta = gun_pos - gun_roll;
    gun_roll = gun_pos;

    static auto gun_pitch = float{};
    const auto pitch_delta = result.final_recoil - gun_pitch;
    gun_pitch = result.final_recoil;

    const auto bob_amount = bob_spring_.update(delta);

    static auto bob = float{};
    const auto bob_delta = bob_amount - bob;
    bob = bob_amount;

    turn_spring_.add_impulse(input_map.delta_x * 0.05f);
    auto gun_transform = entity.local_transform() * Transform{{}, {1.0f}, Quaternion(0.0f, -pitch_delta, -roll_delta)};
    gun_transform.position.y += bob_delta;
    entity.set_transform(gun_transform);

    return result;
}

auto Gun::description() const -> Description
{
    return {.fire_rate = fire_rate_};
}
}
