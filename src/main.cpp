#include <fstream>
#include <memory>
#include <numbers>
#include <ranges>
#include <span>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <variant>

#include <windows.h>

#include <yaml-cpp/yaml.h>

#include "config.h"

#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"
#include "concurrency/thread_pool.h"
#include "core/render_entity.h"
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
#include "resources/resource_loader.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/data_buffer.h"
#include "utils/decompress.h"
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

    constexpr auto speed = 0.1f;
    return ufps::Vector3::normalise(direction) * speed;
}

auto build_mesh_lookup(ufps::ResourceLoader &resource_loader) -> ufps::StringMap<std::vector<ufps::MeshView>>
{
    auto mesh_lookup = ufps::StringMap<std::vector<ufps::MeshView>>{};

    const auto manifest_str = resource_loader.load_string("configs\\model_manifest.yaml");
    const auto manifest = ::YAML::Load(manifest_str);

    for (const auto &model : manifest)
    {
        const auto &name = model.first.as<std::string>();
        const auto &sub_models = model.second;

        if (!sub_models.IsDefined() || !sub_models.IsMap())
        {
            ufps::log::warn("invalid manifest entry for model {}", model.first.as<std::string>());
            continue;
        }

        auto mesh_views = std::vector<ufps::MeshView>{};

        for (const auto &sub_model : sub_models)
        {
            const auto sub_model_name = sub_model.first.as<std::string>();
            const auto sub_model_data = sub_model.second;

            if (!sub_model_data || !sub_model_data["vertex_count"] || !sub_model_data["vertex_offset"] ||
                !sub_model_data["index_count"] || !sub_model_data["index_offset"])
            {
                ufps::log::warn("invalid manifest entry for model {}, submodel {}", name, sub_model_name);
                continue;
            }

            const auto vertex_count = sub_model_data["vertex_count"].as<std::uint32_t>();
            const auto vertex_offset = sub_model_data["vertex_offset"].as<std::uint32_t>();
            const auto index_count = sub_model_data["index_count"].as<std::uint32_t>();
            const auto index_offset = sub_model_data["index_offset"].as<std::uint32_t>();

            mesh_views.push_back(
                {.index_offset = index_offset,
                 .index_count = index_count,
                 .vertex_offset = vertex_offset,
                 .vertex_count = vertex_count});
        }

        mesh_lookup.insert({name, std::move(mesh_views)});
    }

    return mesh_lookup;
}

auto load_texture_from_blob(
    const YAML::Node &texture_manifest,
    const std::vector<std::byte> &texture_blob,
    const std::string &texture_name,
    ufps::ResourceLoader &resource_loader,
    ufps::TextureManager &texture_manager,
    const ufps::Sampler &sampler) -> std::uint32_t
{
    if (!texture_manifest[texture_name] || !texture_manifest[texture_name]["offset"] ||
        !texture_manifest[texture_name]["size"])
    {

        const auto texture_data = resource_loader.load_data_buffer(texture_name);
        auto texture = ufps::load_texture(texture_data, true);
        auto t = ufps::Texture{std::move(texture), texture_name, sampler};
        return texture_manager.add(std::move(t));
    }
    else
    {
        const auto offset = texture_manifest[texture_name]["offset"].as<std::uint64_t>();
        const auto size = texture_manifest[texture_name]["size"].as<std::uint64_t>();

        const auto texture_data = ufps::DataBufferView{
            texture_blob.data() + offset,
            size,
        };
        auto texture = ufps::load_texture(texture_data, true);
        auto t = ufps::Texture{std::move(texture), texture_name, sampler};
        return texture_manager.add(std::move(t));
    }
}

auto build_materials(
    ufps::ResourceLoader &resource_loader,
    ufps::TextureManager &texture_manager,
    ufps::MaterialManager &material_manager,
    const ufps::Sampler &sampler) -> ufps::StringMap<std::vector<std::uint32_t>>
{
    auto material_lookup = ufps::StringMap<std::vector<std::uint32_t>>{};

    const auto model_manifest_str = resource_loader.load_string("configs\\model_manifest.yaml");
    const auto model_manifest = YAML::Load(model_manifest_str);

    const auto texture_blob = ufps::decompress(resource_loader.load_data_buffer("blobs\\texture_data.bin"));
    const auto texture_manifest_str = resource_loader.load_string("configs\\texture_manifest.yaml");
    const auto texture_manifest = YAML::Load(texture_manifest_str);

    for (const auto &model : model_manifest)
    {
        const auto &name = model.first.as<std::string>();
        const auto &sub_models = model.second;

        if (!sub_models.IsDefined() || !sub_models.IsMap())
        {
            ufps::log::warn("invalid manifest entry for model {}", model.first.as<std::string>());
            continue;
        }

        for (const auto &sub_model : sub_models)
        {
            const auto sub_model_name = sub_model.first.as<std::string>();
            const auto sub_model_data = sub_model.second;

            if (!sub_model_data || !sub_model_data["vertex_count"] || !sub_model_data["vertex_offset"] ||
                !sub_model_data["index_count"] || !sub_model_data["index_offset"])
            {
                ufps::log::warn("invalid manifest entry for model {}, submodel {}", name, sub_model_name);
                continue;
            }

            const auto albedo_name = sub_model_data["albedo_name"].as<std::string>();
            const auto albedo_index = albedo_name.empty()
                                          ? 0u
                                          : *texture_manager.try_get_texture_index(albedo_name)
                                                 .or_else(
                                                     [&]
                                                     {
                                                         return std::make_optional(load_texture_from_blob(
                                                             texture_manifest,
                                                             texture_blob,
                                                             albedo_name,
                                                             resource_loader,
                                                             texture_manager,
                                                             sampler));
                                                     });

            const auto normal_name = sub_model_data["normal_name"].as<std::string>();
            const auto normal_index = normal_name.empty()
                                          ? 0u
                                          : *texture_manager.try_get_texture_index(normal_name)
                                                 .or_else(
                                                     [&]
                                                     {
                                                         return std::make_optional(load_texture_from_blob(
                                                             texture_manifest,
                                                             texture_blob,
                                                             normal_name,
                                                             resource_loader,
                                                             texture_manager,
                                                             sampler));
                                                     });

            const auto specular_name = sub_model_data["specular_name"].as<std::string>();
            const auto specular_index = specular_name.empty()
                                            ? 0u
                                            : *texture_manager.try_get_texture_index(specular_name)
                                                   .or_else(
                                                       [&]
                                                       {
                                                           return std::make_optional(load_texture_from_blob(
                                                               texture_manifest,
                                                               texture_blob,
                                                               specular_name,
                                                               resource_loader,
                                                               texture_manager,
                                                               sampler));
                                                       });

            const auto material_index = material_manager.add(albedo_index, normal_index, specular_index);
            material_lookup[name].push_back(material_index);
        }
    }

    return material_lookup;
}

}

auto pulse_light(ufps::AwaitableManager &awaitable, ufps::PointLightHandle handle, ufps::Scene &scene) -> ufps::Task
{
    auto fake_time = 0.0f;

    for (;;)
    {
        if (auto light = scene.lights().lights[handle]; light)
        {
            light->intensity = 5.0f + (10.0f * ((std::sin(fake_time) + 1.0f) / 2.0f));
        }
        else
        {
            ufps::log::info("ending pulse_light coroutine");
            co_return;
        }
        fake_time += 0.1f;

        co_await awaitable;
    }
}

// auto flicker_light(ufps::AwaitableManager &awaitable, ufps::PointLight *light) -> ufps::Task
// {
//     for (;;)
//     {
//         co_await awaitable(3s);
//         light->intensity = 0.0f;
//         co_await awaitable(100ms);
//         light->intensity = 15.0f;
//     }
// }

int start()
{
    // Daz_Da_Cat: First stream done.
    // Daz_Da_Cat: You can't handle the Daz!
    ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ufps::log::info(
        "μfps version: {}.{}.{}.{}",
        ufps::version::year,
        ufps::version::month,
        ufps::version::day,
        ufps::version::tweak);
    ufps::log::info("{}", ufps::system_info());

    auto window = ufps::Window{ufps::WindowMode::WINDOWED, 1920u, 1080u, 1920u, 0u};
    auto running = true;

    auto resource_loader = std::unique_ptr<ufps::ResourceLoader>();
    if constexpr (ufps::config::use_embedded_resouce_loader)
    {
        ufps::log::info("using embedded resource loader");
        resource_loader = std::make_unique<ufps::EmbeddedResourceLoader>();
    }
    else
    {
        ufps::log::info("using file resource loader");
        resource_loader = std::make_unique<ufps::FileResourceLoader>(
            std::vector<std::filesystem::path>{"assets", "secret-assets", "build\\build_assets"});
    }

    const auto sampler = ufps::Sampler{
        ufps::FilterType::LINEAR,
        ufps::FilterType::LINEAR,
        ufps::WrapMode::REPEAT,
        ufps::WrapMode::REPEAT,
        "simple_sampler"};

    auto pool = ufps::ThreadPool{};
    auto awaitable_manager = ufps::AwaitableManager{pool};
    auto mesh_manager = ufps::MeshManager{
        ufps::decompress(resource_loader->load_data_buffer("blobs\\vertex_data.bin")),
        ufps::decompress(resource_loader->load_data_buffer("blobs\\index_data.bin")),
        build_mesh_lookup(*resource_loader)};
    auto material_manager = ufps::MaterialManager{};
    auto texture_manager = ufps::TextureManager{};

    mesh_manager.load("cube", std::vector{cube()});

    auto renderer = ufps::DebugRenderer{window, *resource_loader, texture_manager, mesh_manager};
    auto debug_mode = false;

    auto strm = std::stringstream{};
    auto scene_description_yaml = std::ifstream{"scene.yaml"};

    if (scene_description_yaml.is_open())
    {
        strm << scene_description_yaml.rdbuf();
    }
    else
    {
        if constexpr (ufps::config::use_embedded_resouce_loader)
        {
            auto scene_description_str = resource_loader->load_string("configs\\scene.yaml");
            strm << scene_description_str;
        }
    }

    const auto scene_description = ufps::yaml::deserialise<ufps::Scene::Description>(strm.str());

    const auto material_lookup = build_materials(*resource_loader, texture_manager, material_manager, sampler);

    auto entity_cache = ufps::StringMap<ufps::Entity>{};

    for (const auto &[name, materials] : material_lookup)
    {
        const auto &mesh_views = mesh_manager.mesh(name);
        auto render_entities = std::views::zip(mesh_views, materials) |
                               std::views::transform(
                                   [&mesh_manager](const auto &e)
                                   {
                                       const auto &[mesh_view, material] = e;
                                       return ufps::RenderEntity(mesh_view, material, mesh_manager);
                                   }) |
                               std::ranges::to<std::vector>();
        entity_cache.insert({name, ufps::Entity{name, std::move(render_entities), {}}});
    }

    auto scene = ufps::Scene{
        mesh_manager,
        material_manager,
        texture_manager,
        {{},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 1.0f, 0.0f},
         std::numbers::pi_v<float> / 4.0f,
         static_cast<float>(window.render_width()),
         static_cast<float>(window.render_height()),
         0.1f,
         1000.0f},
        scene_description,
        entity_cache};

    auto key_state = std::unordered_map<ufps::Key, bool>{
        {ufps::Key::W, false}, {ufps::Key::A, false}, {ufps::Key::S, false}, {ufps::Key::D, false}};

    const auto point_light_handles = scene.lights().lights.handles();

    pulse_light(awaitable_manager, point_light_handles[0], scene);
    // flicker_light(awaitable_manager, std::addressof(scene.lights().lights[2]));

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
                        if (!debug_mode || key_state[ufps::Key::SHIFT])
                        {
                            static constexpr auto sensitivity = float{0.002f};
                            const auto delta_x = arg.delta_x() * sensitivity;
                            const auto delta_y = arg.delta_y() * sensitivity;
                            scene.camera().adjust_yaw(delta_x);
                            scene.camera().adjust_pitch(-delta_y);
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

        awaitable_manager.pump();
        pool.drain();

        scene.camera().translate(walk_direction(key_state, scene.camera()));

        renderer.render(scene);

        window.swap();
    }

    awaitable_manager.pump();
    pool.drain();

    return 0;
}

int main()
{
    try
    {
        return start();
    }
    catch (const ufps::Exception &e)
    {
        ufps::log::error("{}", e);
        return -1;
    }
    catch (const std::exception &e)
    {
        ufps::log::error("{}", e.what());
        return -1;
    }
    catch (...)
    {
        ufps::log::error("unhandled unknown exception");
        return -1;
    }
}
