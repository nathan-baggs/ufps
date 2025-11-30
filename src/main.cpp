#include <GL/gl.h>
#include <string_view>
#include <variant>

#include <windows.h>

#include "config.h"
#include "events/key_event.h"
#include "graphics/buffer.h"
#include "graphics/colour.h"
#include "graphics/opengl.h"
#include "graphics/program.h"
#include "graphics/shader.h"
#include "graphics/vertex_data.h"
#include "graphics/window.h"
#include "utils/data_buffer.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/system_info.h"

using namespace std::literals;

namespace
{
constexpr auto sample_vertex_shader = R"(
#version 460 core
    
struct VertexData
{
    float position[3];
    float colour[3];
};

layout(binding = 0, std430) readonly buffer vertices {
    VertexData data[];
};

vec3 get_position(int index)
{
    return vec3(
        data[index].position[0],
        data[index].position[1],
        data[index].position[2]);
}

vec3 get_colour(int index)
{
    return vec3(
        data[index].colour[0],
        data[index].colour[1],
        data[index].colour[2]);
}

layout (location = 0) out vec3 out_colour;

void main()
{
    gl_Position = vec4(get_position(gl_VertexID), 1.0);
    out_colour = get_colour(gl_VertexID);
}
)"sv;

constexpr auto sample_fragment_shader = R"(
#version 460 core

layout(location = 0) in vec3 in_colour;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = vec4(in_colour, 1.0);
}
)"sv;

struct IndirectCommand
{
    std::uint32_t count;
    std::uint32_t instance_count;
    std::uint32_t first;
    std::uint32_t base_instance;
};

}

int main()
{
    // Daz_Da_Cat: First stream done.
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info("μfps version: {}.{}.{}", ufps::version::major, ufps::version::minor, ufps::version::patch);
    ufps::log::info("{}", ufps::system_info());

    auto window = ufps::Window{ufps::WindowMode::WINDOWED, 1920u, 1080u, 1920u, 0u};
    auto running = true;

    const auto sample_vert = ufps::Shader{sample_vertex_shader, ufps::ShaderType::VERTEX, "sample_vertex_shader"sv};
    const auto sample_frag =
        ufps::Shader{sample_fragment_shader, ufps::ShaderType::FRAGMENT, "sample_fragment_shader"sv};
    const auto sample_prog = ufps::Program{sample_vert, sample_frag, "sample_prog"sv};

    ufps::VertexData triangle[] = {
        {{0.0f, 0.5f, 0.0f}, ufps::colours::azure},
        {{-0.5f, -0.5f, 0.0f}, ufps::Colour{0.6, 0.1, 0.0}},
        {{0.5f, -0.5f, 0.0f}, ufps::Colour{0.42, 0.42, 0.42}},
    };

    const auto triangle_view = ufps::DataBufferView{reinterpret_cast<const std::byte *>(triangle), sizeof(triangle)};

    const auto triangle_buffer = ufps::Buffer{sizeof(triangle)};
    triangle_buffer.write(triangle_view, 0zu);

    const auto command_buffer = ufps::Buffer(sizeof(IndirectCommand));
    const auto command = IndirectCommand{
        .count = 3,
        .instance_count = 1,
        .first = 0,
        .base_instance = 0,
    };
    const auto command_view = ufps::DataBufferView{reinterpret_cast<const std::byte *>(&command), sizeof(command)};
    command_buffer.write(command_view, 0zu);

    auto dummy_vao = ufps::AutoRelease<::GLuint>{0u, [](auto e) { ::glDeleteVertexArrays(1, &e); }};
    ::glGenVertexArrays(1, &dummy_vao);

    ::glBindVertexArray(dummy_vao);
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangle_buffer.native_handle());
    ::glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer.native_handle());
    sample_prog.use();

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

        triangle[0].colour.r += 0.01f;
        if (triangle[0].colour.r >= 1.0f)
        {
            triangle[0].colour.r = 0.0f;
        }

        triangle_buffer.write(triangle_view, 0zu);

        ::glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, 1, 0);

        window.swap();
    }

    return 0;
}
