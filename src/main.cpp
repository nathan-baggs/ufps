#include <numbers>
#include <ranges>
#include <string_view>
#include <variant>

#include <windows.h>

#include "config.h"
#include "events/key_event.h"
#include "graphics/colour.h"
#include "graphics/command_buffer.h"
#include "graphics/mesh_data.h"
#include "graphics/mesh_manager.h"
#include "graphics/renderer.h"
#include "graphics/scene.h"
#include "graphics/vertex_data.h"
#include "graphics/window.h"
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
        .vertices = positions |
                    std::views::transform([](const auto &e)
                                          { return ufps::VertexData{.position = e, .colour = ufps::colours::azure}; }) |
                    std::ranges::to<std::vector>(),
        .indices = std::move(indices)};
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
    auto renderer = ufps::Renderer{};

    auto scene = ufps::Scene{
        .entities = {},
        .mesh_manager = mesh_manager,
        .camera = {
            {},
            {0.0f, 0.0f, -1.0f},
            {0.0f, 1.0f, 0.0f},
            std::numbers::pi_v<float> / 4.0f,
            static_cast<float>(window.render_width()),
            static_cast<float>(window.render_height()),
            0.1f,
            1000.0f}};

    scene.entities.push_back({.mesh_view = mesh_manager.load(cube())});

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
                    }
                },
                *event);

            event = window.pump_event();
        }

        scene.camera.translate({0.0f, 0.0f, 0.01f});

        renderer.render(scene);

        window.swap();
    }

    return 0;
}
