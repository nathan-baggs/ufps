#include <string_view>
#include <variant>

#include <windows.h>

#include "config.h"
#include "events/key_event.h"
#include "graphics/opengl.h"
#include "graphics/shader.h"
#include "graphics/window.h"
#include "utils/formatter.h"
#include "utils/hello.h"
#include "utils/log.h"
#include "utils/system_info.h"

using namespace std::literals;

namespace
{
constexpr auto sample_shader = R"(
#version 460 core
void main()
{
    gl_Position = vec4(1.0f);
}
)"sv;
}

int main()
{
    // Daz_Da_Cat: First stream done.
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info("μfps version: {}.{}.{}", ufps::version::major, ufps::version::minor, ufps::version::patch);
    ufps::log::info("{}", ufps::system_info());

    auto window = ufps::Window{ufps::WindowMode::WINDOWED, 1920u, 1080u, 1920u, 0u};
    auto running = true;

    [[maybe_unused]] auto simple_vert = ufps::Shader{sample_shader, ufps::ShaderType::VERTEX, "sample_shader"sv};

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
                        ufps::log::info("stopping");
                        running = false;
                    }
                },
                *event);

            event = window.pump_event();
        }

        window.swap();
    }

    return 0;
}
