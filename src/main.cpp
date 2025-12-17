#include <_mingw_mac.h>
#include <numbers>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <windows.h>

#include "config.h"
#include "events/key.h"
#include "events/key_event.h"
#include "graphics/colour.h"
#include "graphics/debug_ui.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_data.h"
#include "graphics/mesh_manager.h"
#include "graphics/renderer.h"
#include "graphics/scene.h"
#include "graphics/vertex_data.h"
#include "graphics/window.h"
#include "maths/vector3.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/system_info.h"

using namespace std::literals;

namespace
{
auto cube() -> ufps::MeshData
{
    const ufps::Vector3 positions[] = {
        {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, -1.0f},  {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f},
    };

    auto indices = std::vector<std::uint32_t>{
        0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20,
    };

    return {
        .vertices = positions | std::views::transform([](const auto &e) { return ufps::VertexData{.position = e}; }) |
                    std::ranges::to<std::vector>(),
        .indices = std::move(indices)};
}

auto walk_direction(std::unordered_map<ufps::Key, bool> &key_state, const ufps::Camera &camera) -> ufps::Vector3
{
    auto direction = ufps::Vector3{};

    if (key_state[ufps::Key::W])
    {
        direction += camera.direction();
    }

    if (key_state[ufps::Key::S])
    {
        direction -= camera.direction();
    }

    if (key_state[ufps::Key::D])
    {
        direction += camera.right();
    }

    if (key_state[ufps::Key::A])
    {
        direction -= camera.right();
    }

    if (key_state[ufps::Key::Q])
    {
        direction += camera.up();
    }

    if (key_state[ufps::Key::E])
    {
        direction -= camera.up();
    }

    constexpr auto speed = 0.5f;
    return ufps::Vector3::normalise(direction) * speed;
}

}

int main()
{
    // Daz_Da_Cat: First stream done.
    // Daz_Da_Cat: You can't handle the Daz!
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info("μfps version: {}.{}.{}", ufps::version::major, ufps::version::minor, ufps::version::patch);
    ufps::log::info("{}", ufps::system_info());

    auto window = ufps::Window{ufps::WindowMode::WINDOWED, 1920u, 1080u, 1920u, 0u};
    auto running = true;

    auto mesh_manager = ufps::MeshManager{};
    auto material_manager = ufps::MaterialManager{};
    auto renderer = ufps::Renderer{};
    auto debug_ui = ufps::DebugUI{window};
    auto debug_mode = false;

    const auto material_key_red = material_manager.add(ufps::Colour{1.0f, 0.0f, 0.0f});
    const auto material_key_blue = material_manager.add(ufps::Colour{0.0f, 0.0f, 1.0f});
    const auto material_key_green = material_manager.add(ufps::Colour{0.0f, 1.0f, 0.0f});
    material_manager.remove(material_key_blue);

    auto scene = ufps::Scene{
        .entities = {},
        .mesh_manager = mesh_manager,
        .material_manager = material_manager,
        .camera =
            {{},
             {0.0f, 0.0f, -1.0f},
             {0.0f, 1.0f, 0.0f},
             std::numbers::pi_v<float> / 4.0f,
             static_cast<float>(window.render_width()),
             static_cast<float>(window.render_height()),
             0.1f,
             1000.0f},
    };

    scene.entities.push_back({
        .mesh_view = mesh_manager.load(cube()),
        .transform = {{10.0f, 0.0f, -10.0f}, {5.0f}, {}},
        .material_key = material_key_red,
    });

    scene.entities.push_back({
        .mesh_view = mesh_manager.load(cube()),
        .transform = {{-10.0f, 0.0f, -10.0f}, {5.0f}, {}},
        .material_key = material_key_green,
    });

    auto key_state = std::unordered_map<ufps::Key, bool>{
        {ufps::Key::W, false}, {ufps::Key::A, false}, {ufps::Key::S, false}, {ufps::Key::D, false}};

    while (running)
    {
        ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto event = window.pump_event();
        while (event && running)
        {
            std::visit(
                [&](auto &&arg)
                {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::same_as<T, ufps::KeyEvent>)
                    {
                        if (arg.key() == ufps::Key::ESC)
                        {
                            ufps::log::info("stopping");
                            running = false;
                        }
                        if (arg == ufps::KeyEvent{ufps::Key::F1, ufps::KeyState::DOWN})
                        {
                            debug_mode = !debug_mode;
                        }
                        else
                        {
                            key_state[arg.key()] = arg.state() == ufps::KeyState::DOWN;
                        }
                    }
                    else if constexpr (std::same_as<T, ufps::MouseEvent>)
                    {
                        if (!debug_mode)
                        {
                            static constexpr auto sensitivity = float{0.002f};
                            const auto delta_x = arg.delta_x() * sensitivity;
                            const auto delta_y = arg.delta_y() * sensitivity;
                            scene.camera.adjust_yaw(delta_x);
                            scene.camera.adjust_pitch(-delta_y);
                        }
                    }
                    else if constexpr (std::same_as<T, ufps::MouseButtonEvent>)
                    {
                        debug_ui.add_mouse_event(arg);
                    }
                },
                *event);

            event = window.pump_event();
        }

        scene.camera.translate(walk_direction(key_state, scene.camera));

        renderer.render(scene);

        if (debug_mode)
        {
            debug_ui.render(scene);
        }

        window.swap();
    }

    return 0;
}
