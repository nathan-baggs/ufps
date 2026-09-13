#pragma once

#include <vector>

#include "core/camera.h"
#include "core/clock.h"
#include "core/entity.h"
#include "core/scene.h"
#include "events/input_map.h"
#include "maths/ray.h"
#include "maths/spring.h"

namespace ufps
{

class Gun
{
  public:
    struct Description
    {
        Duration fire_rate;
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
    Duration shoot_timer_;
    Duration fire_rate_;
    Spring recoil_spring_;
    Spring turn_spring_;
    Spring bob_spring_;
    float recoil_target_;
};

}
