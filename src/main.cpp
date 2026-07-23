#include <fstream>
#include <map>
#include <memory>
#include <numbers>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <variant>

#include <objbase.h>
#include <windows.h>

#include <yaml-cpp/yaml.h>

#include "config.h"

#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"
#include "concurrency/thread_pool.h"
#include "core/actor.h"
#include "core/flycam_actor.h"
#include "core/manifest_descriptions.h"
#include "core/player_actor.h"
#include "core/render_entity.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "events/key.h"
#include "events/key_event.h"
#include "graphics/colour.h"
#include "graphics/debug_renderer.h"
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
#include "memory/metrics.h"
#include "physics/physics_system.h"
#include "physics/rigid_body.h"
#include "resources/embedded_resource_loader.h"
#include "resources/file_resource_loader.h"
#include "resources/resource_loader.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/data_buffer.h"
#include "utils/decompress.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/resolve_symbols.h"
#include "utils/stack_trace_buffer.h"
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

auto load_all_textures(
    ufps::ResourceLoader &resource_loader,
    ufps::TextureManager &texture_manager,
    const ufps::Sampler &sampler) -> void
{
    const auto texture_manifest_str = resource_loader.load_string("configs\\texture_manifest.yaml");
    const auto texture_manifest = ufps::yaml::deserialise<ufps::TextureManifestDescription>(texture_manifest_str);
    ensure(texture_manifest);

    const auto texture_blob = ufps::decompress(resource_loader.load_data_buffer("blobs\\texture_data.bin"));

    for (const auto &[name, manifest] : texture_manifest->textures)
    {
        const auto raw_texture_data = std::span{texture_blob.data() + manifest.offset, manifest.size};
        const auto texture_data = ufps::load_texture(raw_texture_data, manifest.is_srgb);
        texture_manager.add({texture_data, name, sampler});
    }
}

auto build_mesh_lookup(ufps::ResourceLoader &resource_loader) -> ufps::StringMap<std::vector<ufps::MeshView>>
{
    auto mesh_lookup = ufps::StringMap<std::vector<ufps::MeshView>>{};

    const auto manifest_str = resource_loader.load_string("configs\\model_manifest.yaml");
    const auto manifest = ufps::yaml::deserialise<ufps::ModelManifestDescription>(manifest_str);
    ensure(manifest);

    return manifest->models |
           std::views::transform(
               [](const auto &e)
               {
                   const auto &[name, manifests] = e;
                   return std::pair{
                       name,
                       manifests | std::views::transform([](const auto &m) { return m.mesh_view; }) |
                           std::ranges::to<std::vector>()};
               }) |
           std::ranges::to<ufps::StringMap<std::vector<ufps::MeshView>>>();
}

auto build_entity_cache(ufps::ResourceLoader &resource_loader) -> ufps::StringMap<ufps::Entity>
{
    auto &texture_manager = ufps::service<ufps::TextureManager>();
    auto entity_cache = ufps::StringMap<ufps::Entity>{};

    const auto model_manifest_str = resource_loader.load_string("configs\\model_manifest.yaml");
    const auto model_manifest = ufps::yaml::deserialise<ufps::ModelManifestDescription>(model_manifest_str);
    ensure(model_manifest);

    for (const auto &[name, manifests] : model_manifest->models)
    {
        auto render_entities = std::vector<ufps::RenderEntity>{};

        for (const auto &[mesh_view, albedo, normal, specular, ao, glosiness, emissive] : manifests)
        {
            const auto albedo_index = texture_manager.bindless_handle(albedo);
            const auto normal_index = texture_manager.bindless_handle(normal);
            const auto specular_index = texture_manager.bindless_handle(specular);
            const auto ao_index = texture_manager.bindless_handle(ao);
            const auto glossiness_index = texture_manager.bindless_handle(glosiness);
            const auto emissive_index = texture_manager.bindless_handle(emissive);

            render_entities.push_back(
                {mesh_view, albedo_index, normal_index, specular_index, ao_index, glossiness_index, emissive_index});
        }

        entity_cache.insert({name, ufps::Entity{name, std::move(render_entities), {}}});
    }

    return entity_cache;
}

auto pulse_light(ufps::PointLightHandle handle, ufps::Scene &scene) -> ufps::Task
{
    auto &awaitable = ufps::service<ufps::AwaitableManager>();
    auto fake_time = 0.0f;

    for (;;)
    {
        if (auto light = scene.lights().lights[handle]; light)
        {
            light->intensity = 2.0f + (5.0f * ((std::sin(fake_time) + 1.0f) / 2.0f));
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

auto flicker_light(ufps::PointLightHandle handle, ufps::Scene &scene) -> ufps::Task
{
    auto &awaitable = ufps::service<ufps::AwaitableManager>();

    for (;;)
    {
        co_await awaitable(3s);

        if (auto light = scene.lights().lights[handle]; light)
        {
            light->intensity = 0.0f;
        }
        else
        {
            ufps::log::info("ending flicker_light coroutine");
            co_return;
        }

        co_await awaitable(100ms);

        if (auto light = scene.lights().lights[handle]; light)
        {
            light->intensity = 15.0f;
        }
        else
        {
            ufps::log::info("ending flicker_light coroutine");
            co_return;
        }
    }
}
}

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

    auto window = ufps::Window{ufps::WindowMode::WINDOWED, 3840, 2160, 0u, 0u};
    auto running = true;

    auto input_map = ufps::InputMap{};

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

    auto texture_manager = std::make_unique<ufps::TextureManager>();
    load_all_textures(*resource_loader, *texture_manager, sampler);

    auto pool = std::make_unique<ufps::ThreadPool>();
    auto awaitable_manager = std::make_unique<ufps::AwaitableManager>(*pool);
    auto mesh_manager = std::make_unique<ufps::MeshManager>(
        ufps::decompress(resource_loader->load_data_buffer("blobs\\vertex_data.bin")),
        ufps::decompress(resource_loader->load_data_buffer("blobs\\index_data.bin")),
        build_mesh_lookup(*resource_loader));

    mesh_manager->load("cube", std::vector{cube()});

    auto physics = std::make_unique<ufps::PhysicsSystem>(ufps::DebugRenderMode::ON);
    auto &player_controller = physics->player_controller();

    auto player_actor = ufps::PlayerActor{
        {{0.0f, 2.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 1.0f, 0.0f},
         std::numbers::pi_v<float> / 4.0f,
         static_cast<float>(window.render_width()),
         static_cast<float>(window.render_height()),
         0.1f,
         1000.0f},
        input_map,
        player_controller};

    auto flycam_actor = ufps::FlyCamActor{
        {{0.0f, 2.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 1.0f, 0.0f},
         std::numbers::pi_v<float> / 4.0f,
         static_cast<float>(window.render_width()),
         static_cast<float>(window.render_height()),
         0.1f,
         1000.0f},
        input_map};

    ufps::Actor *current_actor = std::addressof(player_actor);

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

    auto services = std::make_unique<ufps::Services>(
        std::move(awaitable_manager),
        std::move(mesh_manager),
        std::move(physics),
        std::move(texture_manager),
        std::move(pool));
    ufps::set_service(services.get());

    auto renderer = ufps::DebugRenderer{window, *resource_loader};
    auto debug_mode = false;

    auto scene_description = ufps::yaml::deserialise<ufps::Scene::Description>(strm.str());
    ufps::ensure(scene_description);

    auto scene = ufps::Scene{std::move(*scene_description), build_entity_cache(*resource_loader)};

    const auto point_light_handles = scene.lights().lights.handles();

    pulse_light(point_light_handles[0], scene);
    flicker_light(point_light_handles[2], scene);

    while (running)
    {
        auto &awaitable = ufps::service<ufps::AwaitableManager>();
        auto &physics = ufps::service<ufps::PhysicsSystem>();
        auto &pool = ufps::service<ufps::ThreadPool>();

        const auto begin_frame_allocated_bytes = ufps::g_metrics.total_allocated_bytes.load(std::memory_order_relaxed);

        input_map.delta_x = 0.0f;
        input_map.delta_y = 0.0f;

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
                            current_actor = debug_mode ? static_cast<ufps::Actor *>(&flycam_actor)
                                                       : static_cast<ufps::Actor *>(&player_actor);
                        }

                        input_map.set(arg);
                    }
                    else if constexpr (std::same_as<T, ufps::MouseEvent>)
                    {
                        if (!debug_mode || input_map[ufps::Key::SHIFT])
                        {
                            static constexpr auto sensitivity = float{0.002f};
                            input_map.delta_x += arg.delta_x() * sensitivity;
                            input_map.delta_y += arg.delta_y() * sensitivity;
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

        current_actor->update();

        physics.update();

        awaitable.pump();
        pool.drain();

        renderer.render(scene, current_actor->camera());

        window.swap();

        const auto end_frame_allocated_bytes = ufps::g_metrics.total_allocated_bytes.load(std::memory_order_relaxed);
        ufps::g_metrics.frame_allocated_bytes.store(
            end_frame_allocated_bytes - begin_frame_allocated_bytes, std::memory_order_relaxed);
    }

    ufps::service<ufps::AwaitableManager>().pump();
    ufps::service<ufps::ThreadPool>().drain();

    auto profile_data = ufps::service<ufps::ThreadPool>().profile_data();
    for (const auto &[index, thread_data] : std::views::enumerate(profile_data))
    {
        ufps::log::info("thread id: {}", index);

        const auto sorted_data =
            thread_data |
            std::views::transform([](const auto &p) { return std::make_pair(std::get<1>(p), std::get<0>(p)); }) |
            std::ranges::to<std::map<std::size_t, ufps::StackTraceBuffer, std::greater<std::size_t>>>();

        for (auto &data : sorted_data | std::views::take(2))
        {
            auto &stack = const_cast<ufps::StackTraceBuffer &>(std::get<1>(data));
            auto &counter = std::get<0>(data);

            ufps::log::info("{}", counter);
            const auto symbols = ufps::resolve_symbols(stack);

            auto symbol_str = std::stringstream{};
            for (const auto &symbol : symbols)
            {
                symbol_str << symbol << '\n';
            }

            ufps::log::info("{}", symbol_str.str());
        }

        break;
    }

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
