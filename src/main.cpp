#include <GL/gl.h>
#include <string_view>
#include <variant>

#include <windows.h>

#include "config.h"
#include "events/key_event.h"
#include "graphics/colour.h"
#include "graphics/command_buffer.h"
#include "graphics/mesh_manager.h"
#include "graphics/renderer.h"
#include "graphics/scene.h"
#include "graphics/window.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/system_info.h"

using namespace std::literals;

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

    auto scene = ufps::Scene{.entities = {}, .mesh_manager = mesh_manager};

    scene.entities.push_back(
        {.mesh_view = mesh_manager.load(
             {{{0.0f, 0.0f, 0.0f}, ufps::colours::azure},
              {{-0.5f, 0.0f, 0.0f}, ufps::Colour{0.6, 0.1, 0.0}},
              {{-0.5f, 0.5f, 0.0f}, ufps::Colour{0.42, 0.42, 0.42}}})});

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
                        if (arg.key() == ufps::Key::T)
                        {
                            static auto once = false;
                            if (!once)
                            {
                                scene.entities.push_back(
                                    {.mesh_view = mesh_manager.load(
                                         {{{0.0f, 0.0f, 0.0f}, ufps::colours::azure},
                                          {{-0.5f, 0.5f, 0.0f}, ufps::Colour{0.42, 0.42, 0.42}},
                                          {{0.0f, 0.5f, 0.0f}, ufps::Colour{0.6, 0.1, 0.0}}})});
                                once = true;
                            }
                        }
                        else
                        {
                            ufps::log::info("stopping");
                            running = false;
                        }
                    }
                },
                *event);

            event = window.pump_event();
        }

        renderer.render(scene);

        window.swap();
    }

    return 0;
}
