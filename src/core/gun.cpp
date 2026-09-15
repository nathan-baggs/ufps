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

namespace
{

constexpr auto duration_to_angular_frequency(ufps::Duration duration) -> float
{
    constexpr static auto settling_constant = 5.8339f;
    return settling_constant / std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();
}

}

namespace ufps
{

Gun::Gun(Description description)
    : rest_transform_{}
    , rest_transform_set_{false}
    , description_{description}
    , recoil_target_{}
    , shoot_timer_{}
    , fire_rate_{description.fire_rate}
    , recoil_spring_{0.0f, 0.0f, 0.0f, 2.0f * std::numbers::pi_v<float>}
    , yaw_spring_{0.0f, 0.0f, 0.0f, duration_to_angular_frequency(description.yaw_settle_time)}
    , pitch_spring_{0.0f, 0.0f, 0.0f, duration_to_angular_frequency(description.pitch_settle_time)}
    , kick_spring_{0.0f, 0.0f, 0.0f, duration_to_angular_frequency(description.kick_settle_time), 0.5f}
{
}

auto Gun::update(Duration delta, Entity &entity, const InputMap &input_map, const Camera &camera) -> UpdateResult
{
    if (!rest_transform_set_)
    {
        rest_transform_ = entity.local_transform();
        rest_transform_set_ = true;
    }

    auto result = UpdateResult{};

    const auto &[am] = services<AudioManager>();

    shoot_timer_ += delta;

    if (input_map.mouse_down)
    {
        if (shoot_timer_ >= fire_rate_)
        {
            kick_spring_.add_impulse(1.0f);

            shoot_timer_ = {};
            recoil_target_ += description_.shot_recoil;

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

    yaw_spring_.add_impulse(input_map.delta_x * description_.yaw_gain);
    const auto yaw_offset = yaw_spring_.update(delta);

    pitch_spring_.add_impulse(input_map.delta_y * description_.pitch_gain);
    const auto pitch_offset = pitch_spring_.update(delta);

    const auto kick_recoil = kick_spring_.update(delta);

    auto gun_transform =
        Transform{
            {0.0f, 0.0f, description_.kick_distance * kick_recoil},
            {1.0f},
            Quaternion(yaw_offset, -pitch_offset, 0.0f)} *
        rest_transform_;
    entity.set_transform(gun_transform);

    return result;
}

auto Gun::description() const -> Description
{
    return description_;
}
}
