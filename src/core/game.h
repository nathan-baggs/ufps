#pragma once

#include <memory>

#include "core/actor.h"
#include "core/clock.h"
#include "core/flycam_actor.h"
#include "core/player_actor.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "graphics/debug_renderer.h"
#include "graphics/sampler.h"
#include "graphics/window.h"

namespace ufps
{

class Game
{
  public:
    Game();
    ~Game();

    auto run() -> void;

  private:
    auto load_scene() -> void;
    auto frame_start() -> bool;
    auto pump_events() -> bool;
    auto update() -> bool;
    auto process_awaitables() -> bool;
    auto render() -> bool;
    auto frame_end() -> bool;

    Window window_;
    InputMap input_map_;
    Sampler default_sampler_;
    Duration delta_;
    TimePoint start_time_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<DebugRenderer> renderer_;
    std::unique_ptr<PlayerActor> player_actor_;
    std::unique_ptr<FlyCamActor> flycam_actor_;
    Actor *current_actor_;
    bool debug_mode_;
    std::size_t allocation_tracker_;
    std::unique_ptr<Services> services_;
};

}
