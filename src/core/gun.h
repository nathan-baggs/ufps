#pragma once

#include <vector>

#include "core/camera.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/scene.h"
#include "events/input_map.h"
#include "maths/bounded_number.h"
#include "maths/ray.h"
#include "maths/spring.h"
#include "maths/transform.h"

namespace ufps
{

class Gun
{
  public:
    struct Description
    {
        Duration fire_rate;
        BoundedFloat<0.0f, 1.0f> shot_recoil;
        BoundedFloat<0.0f, 2.0f> yaw_gain;
        Duration yaw_settle_time;
        BoundedFloat<0.0f, 2.0f> pitch_gain;
        Duration pitch_settle_time;
        BoundedFloat<0.0f, 0.15f> kick_distance;
        BoundedFloat<0.0f, 0.2f> kick_pitch;
        Duration kick_settle_time;
        BoundedFloat<0.0f, 1.0f> bob_amplitude;
        BoundedFloat<0.0f, 10.0f> bob_frequency;
        Duration bob_settle_time;
    };

    struct UpdateResult
    {
        float final_mouse_y_delta;
        float final_recoil;
        std::vector<Ray> bullets_fired;
    };

    Gun(Description description);

    auto update(Duration delta, Entity &entity, const InputMap &input_map, const Camera &camera) -> UpdateResult;

    auto description() const -> Description;

  private:
    Transform rest_transform_;
    bool rest_transform_set_;
    Description description_;
    float recoil_target_;
    Duration shoot_timer_;
    Duration fire_rate_;
    Spring recoil_spring_;
    Spring yaw_spring_;
    Spring pitch_spring_;
    Spring kick_spring_;
    Spring bob_reset_;
    float bob_elapsed_time_;
};

}
