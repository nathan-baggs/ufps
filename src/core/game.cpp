#include "core/game.h"

#include <memory>
#include <ranges>
#include <sstream>
#include <variant>
#include <vector>

#include "audio/audio_manager.h"
#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"
#include "concurrency/thread_pool.h"
#include "core/actor.h"
#include "core/camera.h"
#include "core/camera_manager.h"
#include "core/clock.h"
#include "core/flycam_actor.h"
#include "core/light_manager.h"
#include "core/manifest_descriptions.h"
#include "core/player_actor.h"
#include "core/render_entity_manager.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/input_map.h"
#include "events/key.h"
#include "events/key_event.h"
#include "events/mouse_button_event.h"
#include "events/mouse_event.h"
#include "graphics/colour.h"
#include "graphics/debug_layer.h"
#include "graphics/debug_renderer.h"
#include "graphics/mesh_manager.h"
#include "graphics/mesh_view.h"
#include "graphics/particle_manager.h"
#include "graphics/renderer.h"
#include "graphics/sampler.h"
#include "graphics/shapes.h"
#include "graphics/texture_manager.h"
#include "graphics/utils.h"
#include "graphics/window.h"
#include "maths/cone.h"
#include "maths/random.h"
#include "maths/ray.h"
#include "maths/vector3.h"
#include "memory/metrics.h"
#include "physics/physics_system.h"
#include "resources/embedded_resource_loader.h"
#include "resources/file_resource_loader.h"
#include "resources/resource_loader.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/decompress.h"
#include "utils/log.h"
#include "utils/resolve_symbols.h"
#include "utils/stack_trace_buffer.h"
#include "utils/string_map.h"

namespace
{

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

auto create_services() -> std::unique_ptr<ufps::Services>
{
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

    auto texture_manager = std::make_unique<ufps::TextureManager>();

    auto pool = std::make_unique<ufps::ThreadPool>();
    auto awaitable_manager = std::make_unique<ufps::AwaitableManager>(*pool);
    auto mesh_manager = std::make_unique<ufps::MeshManager>(
        ufps::decompress(resource_loader->load_data_buffer("blobs\\vertex_data.bin")),
        ufps::decompress(resource_loader->load_data_buffer("blobs\\index_data.bin")),
        build_mesh_lookup(*resource_loader));

    auto physics = std::make_unique<ufps::PhysicsSystem>(ufps::DebugRenderMode::ON);

    auto services = std::make_unique<ufps::Services>(
        std::move(awaitable_manager),
        std::move(mesh_manager),
        std::move(physics),
        std::move(texture_manager),
        std::move(pool),
        std::make_unique<ufps::RenderEntityManager>(),
        std::make_unique<ufps::EntityManager>(),
        std::make_unique<ufps::LightManager>(),
        std::make_unique<ufps::CameraManager>(),
        ufps::CameraHandle{},
        std::make_unique<ufps::DebugLayer>(),
        std::make_unique<ufps::AudioManager>(*resource_loader),
        std::make_unique<ufps::ParticleManager>(),
        std::move(resource_loader));
    ufps::set_service(services.release());

    return services;
}

auto load_all_textures(const ufps::Sampler &sampler) -> void
{
    ufps::log::debug("services: {}", static_cast<void *>(ufps::impl::g_services));
    const auto &[rl, tm] = ufps::services<ufps::ResourceLoader, ufps::TextureManager>();

    const auto texture_manifest_str = rl.load_string("configs\\texture_manifest.yaml");
    const auto texture_manifest = ufps::yaml::deserialise<ufps::TextureManifestDescription>(texture_manifest_str);
    ensure(texture_manifest);

    const auto texture_blob = ufps::decompress(rl.load_data_buffer("blobs\\texture_data.bin"));

    for (const auto &[name, manifest] : texture_manifest->textures)
    {
        const auto raw_texture_data = std::span{texture_blob.data() + manifest.offset, manifest.size};
        const auto texture_data = ufps::load_texture(raw_texture_data, manifest.is_srgb);
        tm.add({texture_data, name, sampler});
    }
}

auto load_render_entity_manager()
{
    auto &&[rem, mm, tm, rl] =
        ufps::services<ufps::RenderEntityManager, ufps::MeshManager, ufps::TextureManager, ufps::ResourceLoader>();

    const auto model_manifest_str = rl.load_string("configs\\model_manifest.yaml");
    const auto model_manifest = ufps::yaml::deserialise<ufps::ModelManifestDescription>(model_manifest_str);
    ensure(model_manifest);

    for (const auto &[name, manifests] : model_manifest->models)
    {
        auto render_entities = std::vector<ufps::RenderEntity>{};

        for (const auto &[mesh_view, albedo, normal, specular, ao, glosiness, emissive] : manifests)
        {
            const auto albedo_index = tm.bindless_handle(albedo);
            const auto normal_index = tm.bindless_handle(normal);
            const auto specular_index = tm.bindless_handle(specular);
            const auto ao_index = tm.bindless_handle(ao);
            const auto glossiness_index = tm.bindless_handle(glosiness);
            const auto emissive_index = tm.bindless_handle(emissive);

            render_entities.push_back(
                {name,
                 mesh_view,
                 albedo_index,
                 normal_index,
                 specular_index,
                 ao_index,
                 glossiness_index,
                 emissive_index});
        }

        rem.register_group(name, std::move(render_entities));
    }

    mm.load("cube", std::vector{ufps::shapes::cube()});
    const auto mesh_views = mm.load("sprite", std::vector{ufps::shapes::sprite()});

    rem.register_group(
        "sprite",
        {{"sprite",
          mesh_views.front(),
          tm.texture_index("textures\\default_BaseColor.dds"),
          tm.texture_index("textures\\default_Normal.dds"),
          tm.texture_index("textures\\default_Metallic.dds"),
          tm.texture_index("textures\\default_AO.dds"),
          tm.texture_index("textures\\default_Roughness.dds"),
          tm.texture_index("textures\\default_Emissive.dds")}});
}

auto load_scene_description() -> ufps::Scene::Description
{
    const auto &[rl] = ufps::services<ufps::ResourceLoader>();

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
            auto scene_description_str = rl.load_string("configs\\scene.yaml");
            strm << scene_description_str;
        }
    }

    auto scene_description = ufps::yaml::deserialise<ufps::Scene::Description>(strm.str());
    ufps::ensure(scene_description);

    return *scene_description;
}

auto load_gun_description() -> ufps::Gun::Description
{
    const auto &[rl] = ufps::services<ufps::ResourceLoader>();

    auto strm = std::stringstream{};
    auto gun_description_yaml = std::ifstream{"gun.yaml"};
    if (gun_description_yaml.is_open())
    {
        strm << gun_description_yaml.rdbuf();
    }
    else
    {
        if constexpr (ufps::config::use_embedded_resouce_loader)
        {
            auto scene_description_str = rl.load_string("configs\\gun.yaml");
            strm << scene_description_str;
        }
    }

    auto gun_description = ufps::yaml::deserialise<ufps::Gun::Description>(strm.str());
    ufps::ensure(gun_description);

    return *gun_description;
}

auto pulse_light(ufps::EntityHandle handle) -> ufps::Task
{
    const auto &[awaitable, em, lm] = ufps::services<ufps::AwaitableManager, ufps::EntityManager, ufps::LightManager>();
    auto fake_time = 0.0f;

    for (;;)
    {
        const auto entity = em[handle];
        if (!entity)
        {
            ufps::log::info("ending pulse_light coroutine");
            co_return;
        }

        const auto light = lm[entity->light()];
        if (!light)
        {
            ufps::log::info("ending pulse_light coroutine");
            co_return;
        }

        entity->set_emissive_strength(2.0f + (5.0f * ((std::sin(fake_time) + 1.0f) / 2.0f)));
        light->intensity = 2.0f + (5.0f * ((std::sin(fake_time) + 1.0f) / 2.0f));

        fake_time += 0.1f;

        co_await awaitable;
    }
}

}

namespace ufps
{

Game::Game()
    : window_{ufps::Window{ufps::WindowMode::WINDOWED, 3840, 2160, 0u, 0u}}
    , input_map_{}
    , default_sampler_{ufps::FilterType::LINEAR, ufps::FilterType::LINEAR, ufps::WrapMode::REPEAT, ufps::WrapMode::REPEAT, "default_sampler"}
    , delta_{}
    , start_time_{}
    , scene_{}
    , renderer_{}
    , player_actor_{}
    , flycam_actor_{}
    , current_actor_{}
    , debug_mode_{false}
    , allocation_tracker_{}
    , services_{create_services()}
{
    load_all_textures(default_sampler_);
    load_render_entity_manager();
}

Game::~Game()
{
    const auto &[am, tp] = services<AwaitableManager, ThreadPool>();

    am.pump();
    tp.drain();

    auto profile_data = tp.profile_data();

    for (const auto &[index, thread_data] : std::views::enumerate(profile_data))
    {
        log::info("thread id: {}", index);

        const auto sorted_data =
            thread_data |
            std::views::transform([](const auto &p) { return std::make_pair(std::get<1>(p), std::get<0>(p)); }) |
            std::ranges::to<std::map<std::size_t, StackTraceBuffer, std::greater<std::size_t>>>();

        for (auto &data : sorted_data | std::views::take(2))
        {
            auto &stack = const_cast<StackTraceBuffer &>(std::get<1>(data));
            auto &counter = std::get<0>(data);

            log::info("{}", counter);
            const auto symbols = resolve_symbols(stack);

            auto symbol_str = std::stringstream{};
            for (const auto &symbol : symbols)
            {
                symbol_str << symbol << '\n';
            }

            log::info("{}", symbol_str.str());
        }

        break;
    }
}

auto Game::run() -> void
{
    load_scene();

    for (;;)
    {
        const auto running =
            frame_start() && pump_events() && update() && process_awaitables() && render() && frame_end();

        if (!running)
        {
            break;
        }
    }
}

auto Game::load_scene() -> void
{
    const auto &[rl, am, lm, em, ps] =
        services<ResourceLoader, AudioManager, LightManager, EntityManager, PhysicsSystem>();

    scene_ = std::make_unique<Scene>(load_scene_description());

    const auto point_light_handles = lm.handles();

    auto alert_light = EntityHandle{};
    for (const auto handle : em.handles())
    {
        const auto entity = em[handle];
        if (!entity)
        {
            continue;
        }

        if (entity->name() == "north_corridor_0_alert_light")
        {
            alert_light = handle;
        }
    }

    pulse_light(alert_light);

    auto gun = Gun{load_gun_description()};

    const auto entity_handles = em.handles();
    auto player_entity_handle = std::ranges::find_if(entity_handles, [&](auto e) { return em[e]->name() == "player"; });
    ensure(player_entity_handle != std::ranges::cend(entity_handles), "no player in scene");
    player_actor_ = std::make_unique<PlayerActor>(
        *player_entity_handle, scene_->gun(), std::move(gun), input_map_, ps.player_controller(), *scene_);

    auto flycam_entity_handle = std::ranges::find_if(entity_handles, [&](auto e) { return em[e]->name() == "flycam"; });
    ensure(flycam_entity_handle != std::ranges::cend(entity_handles), "no flycam in scene");
    flycam_actor_ = std::make_unique<FlyCamActor>(*flycam_entity_handle, input_map_);

    current_actor_ = player_actor_.get();

    renderer_ = std::make_unique<DebugRenderer>(window_, *player_actor_);

    am.play("ToTheSpace.wav", PlayMode::LOOP);
    start_time_ = Clock::now();
}

auto Game::frame_start() -> bool
{
    allocation_tracker_ = g_metrics.total_allocated_bytes.load(std::memory_order_relaxed);

    input_map_.delta_x = 0.0f;
    input_map_.delta_y = 0.0f;

    return true;
}

auto Game::pump_events() -> bool
{
    auto event = window_.pump_event();
    auto quit_key_pressed = false;

    while (event)
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
                        quit_key_pressed = true;
                    }
                    if (arg == ufps::KeyEvent{ufps::Key::F1, ufps::KeyState::DOWN})
                    {
                        debug_mode_ = !debug_mode_;
                        ::ShowCursor(debug_mode_);
                        renderer_->set_enabled(debug_mode_);
                        current_actor_ = debug_mode_ ? static_cast<Actor *>(flycam_actor_.get())
                                                     : static_cast<Actor *>(player_actor_.get());
                        ufps::service<ufps::CameraHandle>() = current_actor_->camera();
                    }

                    input_map_.set(arg);
                    renderer_->add_key_event(arg);
                }
                else if constexpr (std::same_as<T, ufps::MouseEvent>)
                {
                    if (!debug_mode_ || input_map_[ufps::Key::SHIFT])
                    {
                        static constexpr auto sensitivity = 0.002f;
                        input_map_.delta_x += arg.delta_x() * sensitivity;
                        input_map_.delta_y += arg.delta_y() * sensitivity;
                    }
                }
                else if constexpr (std::same_as<T, ufps::MouseButtonEvent>)
                {
                    input_map_.mouse_down = arg.state() == MouseButtonState::DOWN;
                    renderer_->add_mouse_event(arg);
                }
            },
            *event);

        if (quit_key_pressed)
        {
            return false;
        }

        event = window_.pump_event();
    }

    return true;
}

auto Game::update() -> bool
{
    const auto &[ps, pm, am, dl] = services<PhysicsSystem, ParticleManager, AudioManager, DebugLayer>();

    current_actor_->update(delta_);
    ps.update();

    const auto bullets_fired = player_actor_->yield_bullets_fired();

    static const auto textures = std::array<std::string_view, 4zu>{{
        "textures\\bullet_hole1.dds",
        "textures\\bullet_hole2.dds",
        "textures\\bullet_hole3.dds",
        "textures\\bullet_hole4.dds",
    }};

    for (const auto &bullet_ray : bullets_fired)
    {
        am.play(player_actor_->gun().fire_sound_name(), PlayMode::SINGLE, random::rand_real(1.0f, 1.4f));

        auto fire_cone = player_actor_->gun().fire_cone();
        auto fire_cone_colour = colours::white;
        auto max_fire_cone = player_actor_->gun().max_fire_cone();
        const auto max_distance = fire_cone.extent.length();

        if (const auto intersection = ps.cast_ray(bullet_ray); intersection)
        {
            if (intersection->distance < max_distance)
            {
                scene_->add_decal(
                    {intersection->position,
                     {0.05f, 0.01f, 0.05f},
                     Quaternion{{0.0f, 1.0f, 0.0f}, intersection->normal} *
                         Quaternion{{0.0f, 1.0f, 0.0f}, random::rand_real(0.0f, 2.0f * std::numbers::pi_v<float>)}},
                    random::rand_element(textures));

                pm.spawn_sparks(intersection->position, intersection->normal * Vector3{2.0f});
                for (auto i = 0; i < 10; ++i)
                {
                    pm.spawn_sparks(
                        intersection->position,
                        intersection->normal +
                            Vector3{
                                random::rand_real(-0.5f, 0.5f),
                                random::rand_real(-0.5f, 0.5f),
                                random::rand_real(-0.5f, 0.5f)} *
                                Vector3{2.0f});
                }
            }
        }
    }

    if (const auto gun_ray_intersection = ps.cast_ray({fire_cone.origin, fire_cone.extent}); gun_ray_intersection)
    {
        if (gun_ray_intersection->distance <= max_distance)
        {
            fire_cone_colour = colours::yellow;
            fire_cone.extent = Vector3::normalise(fire_cone.extent) * Vector3{gun_ray_intersection->distance * 0.99f};
            max_fire_cone.extent = fire_cone.extent;
        }
    }

    dl.push_cone(fire_cone, fire_cone_colour, true, DebugLayerType::DEFAULT);
    dl.push_cone(fire_cone, fire_cone_colour, false, DebugLayerType::DEBUG);
    dl.push_cone(max_fire_cone, colours::magenta, true, DebugLayerType::DEFAULT);
    dl.push_cone(max_fire_cone, colours::magenta, false, DebugLayerType::DEBUG);

    return true;
}

auto Game::process_awaitables() -> bool
{
    const auto &[am, tp] = services<AwaitableManager, ThreadPool>();

    am.pump();
    tp.drain();

    return true;
}

auto Game::render() -> bool
{
    renderer_->render(*scene_, delta_);
    window_.swap();

    return true;
}

auto Game::frame_end() -> bool
{
    if (!debug_mode_)
    {
        ::SetCursorPos(
            static_cast<int>(window_.window_width() / 2.0f), static_cast<int>(window_.window_height() / 2.0f));
    }

    const auto end_frame_allocated_bytes = g_metrics.total_allocated_bytes.load(std::memory_order_relaxed);
    g_metrics.frame_allocated_bytes.store(end_frame_allocated_bytes - allocation_tracker_, std::memory_order_relaxed);

    delta_ = std::chrono::duration_cast<Duration>(Clock::now() - start_time_);
    start_time_ = Clock::now();

    return true;
}
}
