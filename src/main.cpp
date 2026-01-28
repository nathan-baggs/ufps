#include <memory>
#include <numbers>
#include <ranges>
#include <span>
#include <string_view>
#include <unordered_map>
#include <variant>

#include <windows.h>

#include "config.h"
#include "core/scene.h"
#include "events/key.h"
#include "events/key_event.h"
#include "graphics/colour.h"
#include "graphics/debug_renderer.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_data.h"
#include "graphics/mesh_manager.h"
#include "graphics/renderer.h"
#include "graphics/sampler.h"
#include "graphics/texture.h"
#include "graphics/texture_manager.h"
#include "graphics/utils.h"
#include "graphics/vertex_data.h"
#include "graphics/window.h"
#include "maths/vector3.h"
#include "resources/embedded_resource_loader.h"
#include "resources/file_resource_loader.h"
#include "utils/data_buffer.h"
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

    const ufps::Vector3 normals[] = {{0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, 1.0f},
                                     {0.0f, 0.0f, 1.0f},  {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
                                     {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f},
                                     {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
                                     {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f, 0.0f},
                                     {1.0f, 0.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f},  {0.0f, 1.0f, 0.0f},  {0.0f, -1.0f, 0.0f},
                                     {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};

    const ufps::UV uvs[] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
    };

    auto indices = std::vector<std::uint32_t>{
        0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
        12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20,
    };

    auto vs =
        ufps::MeshData{.vertices = vertices(positions, normals, normals, normals, uvs), .indices = std::move(indices)};

    for (const auto &indices : std::views::chunk(vs.indices, 3))
    {
        auto &v0 = vs.vertices[indices[0]];
        auto &v1 = vs.vertices[indices[1]];
        auto &v2 = vs.vertices[indices[2]];

        const auto edge1 = v1.position - v0.position;
        const auto edge2 = v2.position - v0.position;

        const auto deltaUV1 = ufps::UV{.s = v1.uv.s - v0.uv.s, .t = v1.uv.t - v0.uv.t};
        const auto deltaUV2 = ufps::UV{.s = v2.uv.s - v0.uv.s, .t = v2.uv.t - v0.uv.t};

        const auto f = 1.0f / (deltaUV1.s * deltaUV2.t - deltaUV2.s * deltaUV1.t);

        const auto tangent = ufps::Vector3{
            f * (deltaUV2.t * edge1.x - deltaUV1.t * edge2.x),
            f * (deltaUV2.t * edge1.y - deltaUV1.t * edge2.y),
            f * (deltaUV2.t * edge1.z - deltaUV1.t * edge2.z),
        };

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    for (auto &v : vs.vertices)
    {
        v.tangent = ufps::Vector3::normalise(v.tangent - v.normal * ufps::Vector3::dot(v.normal, v.tangent));
        v.bitangent = ufps::Vector3::normalise(ufps::Vector3::cross(v.normal, v.tangent));
    }

    return vs;
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

    std::unique_ptr<ufps::ResourceLoader> resource_loader = std::make_unique<ufps::EmbeddedResourceLoader>();
    auto textures = std::vector<ufps::Texture>{};

    const auto diamond_floor_albedo_data = resource_loader->load_data_buffer("textures\\diamond_floor_albedo.png");
    const auto diamond_floor_albedo = ufps::load_texture(diamond_floor_albedo_data);
    const auto sampler = ufps::Sampler{ufps::FilterType::LINEAR, ufps::FilterType::LINEAR, "simple_sampler"};
    textures.push_back(ufps::Texture{diamond_floor_albedo, "diamond_floor_albedo", sampler});

    const auto diamond_floor_normal_data = resource_loader->load_data_buffer("textures\\diamond_floor_normal.png");
    const auto diamond_floor_normal = ufps::load_texture(diamond_floor_normal_data);
    textures.push_back(ufps::Texture{diamond_floor_normal, "diamond_floor_normal", sampler});

    const auto diamond_floor_specular_data = resource_loader->load_data_buffer("textures\\diamond_floor_specular.png");
    const auto diamond_floor_specular = ufps::load_texture(diamond_floor_specular_data);
    textures.push_back(ufps::Texture{diamond_floor_specular, "diamond_floor_specular", sampler});

    auto mesh_manager = ufps::MeshManager{};
    auto material_manager = ufps::MaterialManager{};
    auto texture_manager = ufps::TextureManager{};

    const auto tex_index = texture_manager.add(std::move(textures));
    ufps::log::debug("tex_index: {}", tex_index);

    auto renderer = ufps::DebugRenderer{window, *resource_loader, texture_manager, mesh_manager};
    auto debug_mode = false;

    const auto material_index_red = material_manager.add(tex_index, tex_index + 1u, tex_index + 2u);
    const auto material_index_blue = material_manager.add(tex_index, tex_index + 1u, tex_index + 2u);
    const auto material_index_green = material_manager.add(tex_index, tex_index + 1u, tex_index + 2u);

    const auto models = ufps::load_model(resource_loader->load_data_buffer("models\\SM_Corner01_8_8_X.fbx"));

    auto scene = ufps::Scene{
        .entities = {},
        .mesh_manager = mesh_manager,
        .material_manager = material_manager,
        .texture_manager = texture_manager,
        .camera =
            {{},
             {0.0f, 0.0f, -1.0f},
             {0.0f, 1.0f, 0.0f},
             std::numbers::pi_v<float> / 4.0f,
             static_cast<float>(window.render_width()),
             static_cast<float>(window.render_height()),
             0.1f,
             1000.0f},
        .lights = {
            .ambient = ufps::Colour{.r = 0.5f, .g = 0.5f, .b = 0.5f},
            .light = {
                .position = {},
                .colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f},
                .constant_attenuation = 1.0f,
                .linear_attenuation = 0.007f,
                .quadratic_attenuation = 0.0002f,
                .specular_power = 32.0f}}};

    scene.entities = models | std::views::enumerate |
                     std::views::transform(
                         [&](const auto &e)
                         {
                             const auto &[index, model] = e;
                             return ufps::Entity{
                                 .name = std::format("model{}", index),
                                 .mesh_view = mesh_manager.load(model.mesh_data),
                                 .transform = {{}, {1.0f}, {}},
                                 .material_index = material_index_red};
                         }) |
                     std::ranges::to<std::vector>();

    auto key_state = std::unordered_map<ufps::Key, bool>{
        {ufps::Key::W, false}, {ufps::Key::A, false}, {ufps::Key::S, false}, {ufps::Key::D, false}};

    while (running)
    {
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
                            renderer.set_enabled(debug_mode);
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
                        renderer.add_mouse_event(arg);
                    }
                },
                *event);

            event = window.pump_event();
        }

        scene.camera.translate(walk_direction(key_state, scene.camera));

        renderer.render(scene);

        window.swap();
    }

    return 0;
}
