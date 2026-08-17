#include "graphics/debug_renderer.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <format>
#include <fstream>
#include <memory>
#include <meta>
#include <optional>
#include <ranges>
#include <string>

#include <imgui.h>

#include <ImGuizmo.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <utility>
#include <variant>
#include <windows.h>

#include "core/camera_manager.h"
#include "core/entity_manager.h"
#include "core/light_manager.h"
#include "core/render_entity_manager.h"
#include "core/scene.h"
#include "core/service_locator.h"
#include "events/key.h"
#include "events/mouse_button_event.h"
#include "graphics/colour.h"
#include "graphics/line_data.h"
#include "graphics/mesh_manager.h"
#include "graphics/opengl.h"
#include "graphics/point_light.h"
#include "graphics/texture_manager.h"
#include "graphics/utils.h"
#include "graphics/window.h"
#include "maths/aabb.h"
#include "maths/bounded_number.h"
#include "maths/matrix4.h"
#include "maths/ray.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
#include "memory/metrics.h"
#include "physics/physics_debug_renderer.h"
#include "physics/physics_system.h"
#include "serialisation/yaml_serialiser.h"
#include "utils/log.h"

using namespace std::literals;

namespace
{

static constexpr auto debug_light_scale = 0.25f;

auto screen_ray(const ufps::MouseButtonEvent &evt, const ufps::Window &window, const ufps::Camera &camera) -> ufps::Ray
{
    const auto x = 2.0f * evt.x() / window.render_width() - 1.0f;
    const auto y = 1.0f - 2.0f * evt.y() / window.render_height();
    const auto ray_clip = ufps::Vector4{x, y, -1.0f, 1.0f};

    const auto inv_proj = ufps::Matrix4::invert(camera.data().projection);
    auto ray_eye = inv_proj * ray_clip;
    ray_eye.z = -1.0f;
    ray_eye.w = 0.0f;

    const auto inv_view = ufps::Matrix4::invert(camera.data().view);
    const auto dir_ws = ufps::Vector3::normalise(ufps::Vector3{inv_view * ray_eye});
    const auto origin_ws = ufps::Vector3{inv_view[12], inv_view[13], inv_view[14]};

    return {origin_ws, dir_ws};
}

auto draw_line(
    const ufps::Vector3 &start,
    const ufps::Vector3 &end,
    const ufps::Colour &colour,
    std::vector<ufps::LineData> &lines) -> void
{
    lines.push_back({start, colour});
    lines.push_back({end, colour});
}

auto create_aabb_lines(const ufps::AABB &aabb, const ufps::Matrix4 &transform, const ufps::Colour &colour)
    -> std::vector<ufps::LineData>
{
    auto lines = std::vector<ufps::LineData>{};

    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        colour,
        lines);

    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        lines);

    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        colour,
        lines);
    draw_line(
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        transform * ufps::Vector4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        colour,
        lines);

    return lines;
}

auto draw_frustum(
    const ufps::Vector3 &cam_pos,
    const ufps::Matrix4 &view,
    const ufps::Matrix4 &proj,
    const ufps::Colour &colour,
    float debug_distance = 3.0f) -> std::vector<ufps::LineData>
{
    auto lines = std::vector<ufps::LineData>{};

    const auto inv_vp = ufps::Matrix4::invert(proj * view);

    constexpr auto ndc_far_corners = std::array<ufps::Vector4, 4>{{
        {-1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f},
        {-1.0f, -1.0f, 1.0f, 1.0f},
    }};

    auto far_corners = std::array<ufps::Vector3, 4>{};
    for (auto i = 0zu; i < 4zu; ++i)
    {
        const auto world_h = inv_vp * ndc_far_corners[i];
        const auto world_pos = ufps::Vector3(world_h) / world_h.w;
        const auto ray_dir = ufps::Vector3::normalise(world_pos - cam_pos);

        far_corners[i] = cam_pos + ray_dir * debug_distance;
    }

    for (auto i = 0zu; i < 4zu; ++i)
    {
        draw_line(cam_pos, far_corners[i], colour, lines);
    }

    for (auto i = 0zu; i < 4zu; ++i)
    {
        draw_line(far_corners[i], far_corners[(i + 1) % 4], colour, lines);
    }

    return lines;
}

template <float Min, float Max>
auto create_debug_controller(const std::string &label, ufps::BoundedFloat<Min, Max> &value) -> void
{
    ::ImGui::SliderFloat(label.c_str(), &value, Min, Max);
}

template <std::uint32_t Min, std::uint32_t Max>
auto create_debug_controller(const std::string &label, ufps::BoundedUint32<Min, Max> &value) -> void
{
    auto v = static_cast<int>(*value);

    if (::ImGui::SliderInt(label.c_str(), &v, Min, Max))
    {
        value = static_cast<std::uint32_t>(v);
    }
}

auto create_debug_controller(const std::string &label, ufps::Colour &value) -> void
{
    float v[3]{};
    std::memcpy(v, &value, sizeof(v));

    if (::ImGui::ColorPicker3(label.c_str(), v))
    {
        std::memcpy(&value, v, sizeof(value));
    }
}

auto create_debug_controller(const std::string &, const ufps::Matrix4 &value) -> void
{
    ::ImGui::BeginTable(
        "transform", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);

    for (auto row = 0; row < 4; ++row)
    {
        ::ImGui::TableNextRow();
        for (auto col = 0; col < 4; ++col)
        {
            ::ImGui::TableSetColumnIndex(col);
            ::ImGui::Text("%0.2f", value[col * 4 + row]);
        }
    }

    ::ImGui::EndTable();
}

}

namespace ufps
{
DebugRenderer::DebugRenderer(const Window &window, ResourceLoader &resource_loader)
    : Renderer{window, resource_loader}
    , enabled_{false}
    , snap_enabled_{false}
    , click_{}
    , selected_{std::monostate{}}
    , debug_lines_{}
    , debug_line_buffer_{sizeof(LineData) * 2u, "line_data_buffer"}
    , debug_line_program_{create_program(
          resource_loader,
          "shaders\\line.vert",
          "line_vertex_shader",
          "shaders\\line.frag",
          "line_fragment_shader",
          "line_program")}
    , debug_light_program_{create_program(
          resource_loader,
          "shaders\\debug_light.vert",
          "debug_light_vertex_shader",
          "shaders\\debug_light.frag",
          "debug_light_fragment_shader",
          "debug_light_program")}
    , highlight_render_entity_{}
{
    IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    auto &io = ::ImGui::GetIO();

    io.ConfigFlags |= ::ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ::ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ::ImGuiConfigFlags_DockingEnable;
    ::ShowCursor(true);
    io.MouseDrawCursor = io.WantCaptureMouse;

    ::ImGui::StyleColorsDark();

    ::ImGui_ImplWin32_InitForOpenGL(window.native_handle());
    ::ImGui_ImplOpenGL3_Init();
}

DebugRenderer::~DebugRenderer()
{
    ::ImGui_ImplOpenGL3_Shutdown();
    ::ImGui_ImplWin32_Shutdown();
    ::ImGui::DestroyContext();
}

auto DebugRenderer::post_render(Scene &scene, const Camera &camera) -> void
{
    auto &&[em, rem, mm, lm, ps, cm] =
        services<EntityManager, RenderEntityManager, MeshManager, LightManager, PhysicsSystem, CameraManager>();

    if (std::holds_alternative<EntityHandle>(selected_))
    {
        const auto selected_entity = std::get<EntityHandle>(selected_);
        auto entity = em[selected_entity];
        contract_assert(entity);

        auto aabb_lines = entity->render_entities() |
                          std::views::transform(
                              [&](auto e)
                              {
                                  auto render_entity = rem[e];
                                  const auto colour =
                                      e == highlight_render_entity_ ? colours::magenta : Colour{0.0f, 0.2f, 0.0f};
                                  return create_aabb_lines(render_entity->aabb(), entity->transform(), colour);
                              }) |
                          std::views::join;

        debug_lines_.append_range(aabb_lines);

        const auto colour = selected_entity == highlight_entity_ ? colours::magenta : colours::green;
        debug_lines_.append_range(create_aabb_lines(entity->aabb(), entity->transform(), colour));
    }

    if (highlight_entity_)
    {
        auto entity = em[highlight_entity_];
        contract_assert(entity);
        debug_lines_.append_range(create_aabb_lines(entity->aabb(), entity->transform(), colours::magenta));
    }

    if (highlight_rigid_body_)
    {
        const auto rb = ps.rigid_body(highlight_rigid_body_);
        contract_assert(rb);
        debug_lines_.append_range(
            create_aabb_lines({.min = {-1.0f}, .max = {1.0f}}, rb->transform(), colours::magenta));
    }

    Renderer::post_render(scene, camera);

    if (!enabled_)
    {
        return;
    }

    light_pass_rt_.fb.unbind();
    ::glBlitNamedFramebuffer(
        gbuffer_rt_.fb.native_handle(),
        0,
        0u,
        0u,
        gbuffer_rt_.fb.width(),
        gbuffer_rt_.fb.height(),
        0u,
        0u,
        gbuffer_rt_.fb.width(),
        gbuffer_rt_.fb.height(),
        GL_DEPTH_BUFFER_BIT,
        GL_NEAREST);

    debug_light_program_.bind();

    const auto [vertex_buffer_handle, index_buffer_handle] = mm.native_handle();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        1,
        camera_buffer_.native_handle(),
        camera_buffer_.frame_offset_bytes(),
        sizeof(CameraData));
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);

    const auto cube_parts = mm.mesh("cube");
    ensure(cube_parts.size() == 1u, "cube mesh should have exactly 1 part");
    const auto cube_indices_offset_bytes = cube_parts.front().index_offset * sizeof(std::uint32_t);
    const auto cube_vertex_offset = cube_parts.front().vertex_offset;

    for (const auto &light : lm.data())
    {
        const auto light_transform = Transform{light.position, {debug_light_scale}, {}};
        const auto light_model = Matrix4{light_transform};

        const auto debug_light_aabb = ufps::AABB{
            .min = light_model * Vector4{-1.0f, -1.0f, -1.0f, 1.0f},
            .max = light_model * Vector4{1.0f},
        };
        debug_lines_.append_range(create_aabb_lines(debug_light_aabb, {}, {1.0f, 0.0f, 0.0f}));

        debug_light_program_.set_uniforms(light_model, light.colour);

        ::glDrawElementsBaseVertex(
            GL_TRIANGLES,
            36,
            GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(cube_indices_offset_bytes),
            cube_vertex_offset);
    }

    for (const auto entity_handle : scene.entities())
    {
        const auto entity = em[entity_handle];
        if (!entity)
        {
            continue;
        }

        if (entity->name() == "flycam")
        {
            continue;
        }

        const auto light_transform = Transform{entity->transform().position, {debug_light_scale / 4.0f}, {}};
        const auto light_model = Matrix4{light_transform};

        debug_light_program_.set_uniforms(light_model, colours::yellow);

        ::glDrawElementsBaseVertex(
            GL_TRIANGLES,
            36,
            GL_UNSIGNED_INT,
            reinterpret_cast<const void *>(cube_indices_offset_bytes),
            cube_vertex_offset);

        const auto camera = cm[entity->camera()];
        if (camera)
        {
            const auto light_transform =
                Transform{camera->transform().position, {debug_light_scale / 4.0f}, camera->transform().rotation};
            const auto light_model = Matrix4{light_transform};

            debug_light_program_.set_uniforms(light_model, colours::azure);

            const auto camera_data = camera->data();

            debug_lines_.append_range(
                draw_frustum(camera_data.position, camera_data.view, camera_data.projection, colours::azure, 1.5f));

            ::glDrawElementsBaseVertex(
                GL_TRIANGLES,
                36,
                GL_UNSIGNED_INT,
                reinterpret_cast<const void *>(cube_indices_offset_bytes),
                cube_vertex_offset);
        }
    }

    debug_light_program_.unbind();

    auto &&physics_debug_renderer = ps.debug_renderer();
    if (physics_debug_renderer)
    {
        debug_lines_.append_range(physics_debug_renderer->yield_lines());
    }

    if (!debug_lines_.empty())
    {
        debug_line_program_.bind();

        resize_gpu_buffer(debug_lines_, debug_line_buffer_);
        debug_line_buffer_.write(std::as_bytes(std::span{debug_lines_.data(), debug_lines_.size()}), 0zu);
        ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, debug_line_buffer_.native_handle());
        ::glBindBufferRange(
            GL_SHADER_STORAGE_BUFFER,
            1,
            camera_buffer_.native_handle(),
            camera_buffer_.frame_offset_bytes(),
            sizeof(CameraData));
        ::glDrawArrays(GL_LINES, 0, debug_lines_.size());

        debug_lines_.clear();

        debug_line_buffer_.advance();
        debug_line_program_.unbind();
    }

    auto &io = ::ImGui::GetIO();

    ::ImGui_ImplOpenGL3_NewFrame();
    ::ImGui_ImplWin32_NewFrame();
    ::ImGui::NewFrame();

    ::ImGuizmo::SetOrthographic(false);
    ::ImGuizmo::BeginFrame();
    ::ImGuizmo::Enable(true);
    ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ::ImGui::DockSpaceOverViewport(0, ::ImGui::GetMainViewport(), ::ImGuiDockNodeFlags_PassthruCentralNode);

    draw_post_processing_window(scene);
    draw_scene(scene, camera);
    draw_logs();
    draw_bloom_mips();
    draw_render_targets();
    draw_metrics();
    draw_inspector(scene);
    draw_gizmo(camera);

    static auto first = false;
    if (!first)
    {
        ::ImGui::SetWindowFocus("Render Targets");
        ::ImGui::SetWindowFocus("Scene");
        first = true;
    }

    ::ImGui::Render();
    ::ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

    if (click_)
    {
        const auto pick_ray = screen_ray(*click_, window_, camera);
        auto intersection = scene.intersect_ray(pick_ray);
        if (intersection)
        {
            selected_ = intersection->entity;
        }
        else
        {
            selected_ = std::monostate{};
        }

        for (const auto &light_handle : lm.handles())
        {
            const auto light = lm[light_handle];

            if (!light)
            {
                continue;
            }

            const auto light_transform = Transform{light->position, {debug_light_scale}, {}};
            const auto light_model = Matrix4{light_transform};

            const auto debug_light_aabb = ufps::AABB{
                .min = light_model * Vector4{-1.0f, -1.0f, -1.0f, 1.0f},
                .max = light_model * Vector4{1.0f},
            };

            if (const auto light_intersection = intersect(pick_ray, debug_light_aabb); light_intersection)
            {
                if (!intersection || light_intersection < intersection->distance)
                {
                    selected_ = light_handle;
                }
            }
        }

        for (const auto entity_handle : scene.entities())
        {
            const auto entity = em[entity_handle];
            if (!entity)
            {
                continue;
            }

            if (entity->name() == "flycam")
            {
                continue;
            }

            const auto entity_transform = Transform{entity->transform().position, {debug_light_scale / 4.0f}, {}};
            const auto entity_model = Matrix4{entity_transform};

            const auto debug_entity_aabb = ufps::AABB{
                .min = entity_model * Vector4{-1.0f, -1.0f, -1.0f, 1.0f},
                .max = entity_model * Vector4{1.0f},
            };

            if (const auto entity_intersection = intersect(pick_ray, debug_entity_aabb); entity_intersection)
            {
                if (!intersection || entity_intersection < intersection->distance)
                {
                    selected_ = entity_handle;
                }
            }

            const auto camera = cm[entity->camera()];
            if (camera)
            {
                const auto camera_transform = Transform{camera->transform().position, {debug_light_scale / 4.0f}, {}};
                const auto camera_model = Matrix4{camera_transform};
                const auto debug_entity_aabb = ufps::AABB{
                    .min = camera_model * Vector4{-1.0f, -1.0f, -1.0f, 1.0f},
                    .max = camera_model * Vector4{1.0f},
                };

                if (const auto camera_intersection = intersect(pick_ray, debug_entity_aabb); camera_intersection)
                {
                    if (!intersection || camera_intersection < intersection->distance)
                    {
                        selected_ = entity->camera();
                    }
                }
            }
        }

        click_.reset();
    }
}

auto DebugRenderer::add_mouse_event(const MouseButtonEvent &evt) -> void
{
    auto &io = ::ImGui::GetIO();
    io.AddMouseButtonEvent(0, evt.state() == MouseButtonState::DOWN);

    if (!io.WantCaptureMouse)
    {
        click_ = evt;
    }
}

auto DebugRenderer::add_key_event(const KeyEvent &evt) -> void
{
    auto &io = ::ImGui::GetIO();

    if (evt.state() == KeyState::DOWN)
    {
        ::BYTE key_state[256] = {};
        ::GetKeyboardState(key_state);

        const auto scan_code = ::MapVirtualKeyA(std::to_underlying(evt.key()), MAPVK_VK_TO_VSC);
        auto chr = ::WORD{};
        if (::ToAscii(std::to_underlying(evt.key()), scan_code, key_state, &chr, 0) == 1)
        {
            io.AddInputCharacter(chr);
        }
    }

    switch (evt.key())
    {
        using enum Key;

        case BACKSPACE: io.AddKeyEvent(::ImGuiKey_Backspace, evt.state() == KeyState::DOWN); break;
        default: break;
    }
}

auto DebugRenderer::set_enabled(bool enabled) -> void
{
    enabled_ = enabled;
    enable_post_processing_ = !enabled_;
}

auto DebugRenderer::draw_post_processing_window(Scene &scene) -> void
{
    if (::ImGui::Begin("Post Processing"))
    {
        ::ImGui::PushID("post_processing");

        ::ImGui::Checkbox("enable post processing", std::addressof(enable_post_processing_));

        if (::ImGui::CollapsingHeader("Tone Map Options"))
        {
            ::ImGui::PushID("tone_map_options");

            create_debug_controller("max_brightness", scene.tone_map_options().max_brightness);
            create_debug_controller("contrast", scene.tone_map_options().contrast);
            create_debug_controller("linear_section_start", scene.tone_map_options().linear_section_start);
            create_debug_controller("linear_section_length", scene.tone_map_options().linear_section_length);
            create_debug_controller("black_tightness", scene.tone_map_options().black_tightness);
            create_debug_controller("pedestal", scene.tone_map_options().pedestal);
            create_debug_controller("gamma", scene.tone_map_options().gamma);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("SSAO Options"))
        {
            ::ImGui::PushID("ssao_options");

            ::ImGui::Checkbox("enabled", std::addressof(scene.ssao_options().enabled));
            create_debug_controller("sample_count", scene.ssao_options().sample_count);
            create_debug_controller("radius", scene.ssao_options().radius);
            create_debug_controller("bias", scene.ssao_options().bias);
            create_debug_controller("power", scene.ssao_options().power);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("Bloom Options"))
        {
            ::ImGui::PushID("bloom_options");

            create_debug_controller("filter_radius", scene.bloom_options().filter_radius);
            create_debug_controller("mix_amount", scene.bloom_options().mix_amount);
            create_debug_controller("threshold", scene.bloom_options().threshold);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("Fog Options"))
        {
            ::ImGui::PushID("fog_options");

            create_debug_controller("colour", scene.fog_options().colour);
            create_debug_controller("density", scene.fog_options().density);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("Chromatci Aberration Options"))
        {
            ::ImGui::PushID("chromatic_aberration_options");

            create_debug_controller("red", scene.chromatic_aberration_options().red_offset);
            create_debug_controller("green", scene.chromatic_aberration_options().green_offset);
            create_debug_controller("blue", scene.chromatic_aberration_options().blue_offset);
            create_debug_controller("strength", scene.chromatic_aberration_options().strength);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("Vignette Options"))
        {
            ::ImGui::PushID("vignette_options");

            create_debug_controller("colour", scene.vignette_options().colour);
            create_debug_controller("strength", scene.vignette_options().strength);
            create_debug_controller("feather", scene.vignette_options().feather);

            ::ImGui::PopID();
        }

        if (::ImGui::CollapsingHeader("Film Grain Options"))
        {
            ::ImGui::PushID("film_grain_options");

            create_debug_controller("strength", scene.film_grain_options().strength);

            ::ImGui::PopID();
        }

        ::ImGui::PopID();
    }
    ::ImGui::End();
}

auto DebugRenderer::draw_scene(Scene &scene, const Camera &camera) -> void
{
    auto &em = service<EntityManager>();

    if (::ImGui::Begin("Scene"))
    {
        ::ImGui::PushID("scene");

        if (::ImGui::Button("Save"))
        {
            const auto scene_yaml = ufps::yaml::serialise(scene.description());
            contract_assert(scene_yaml);
            auto out = std::ofstream("scene.yaml");

            out << *scene_yaml;
        }

        if (::ImGui::Button("Add Entity"))
        {
            static auto counter = std::ranges::size(scene.entities());
            const auto handle = em.insert({std::format("entity_{}", counter++), {}, {}});
            scene.add(handle);
            selected_ = handle;
        }

        ::ImGui::Text("ambient");
        create_debug_controller("ambient", scene.ambient_light());
        ::ImGui::Text("camera_view");
        create_debug_controller("camera_view", camera.data().view);

        ::ImGui::Text("entities");
        const auto entities = scene.entities() |
                              std::views::transform(
                                  [&](const auto &e)
                                  {
                                      const auto entity = em[e];
                                      contract_assert(entity);
                                      return std::make_tuple(entity->name(), e);
                                  }) |
                              std::ranges::to<std::vector>();

        if (::ImGui::BeginListBox("entities"))
        {
            static auto selected_index = std::optional<std::size_t>{};

            auto to_highlight = EntityHandle{};

            for (const auto &[index, entity] : std::views::enumerate(entities))
            {
                const auto &[name, handle] = entity;
                const auto is_selected = selected_index == index;
                const auto flags = is_selected ? ::ImGuiSelectableFlags_Highlight : 0;

                if (::ImGui::Selectable(name.data(), is_selected, flags))
                {
                    selected_index = index;
                    selected_ = handle;
                }

                if (::ImGui::IsItemHovered())
                {
                    to_highlight = handle;
                }

                if (is_selected)
                {
                    ::ImGui::SetItemDefaultFocus();
                }
            }

            highlight_entity_ = to_highlight;

            ::ImGui::EndListBox();
        }

        auto average_luminance = 0.0f;
        ::glGetNamedBufferSubData(
            average_luminance_buffer_.native_handle(), 0, sizeof(average_luminance), &average_luminance);

        std::uint32_t histogram[256]{};
        ::glGetNamedBufferSubData(luminance_histogram_buffer_.native_handle(), 0, sizeof(histogram), &histogram);

        const auto scaled_histogram =
            histogram | std::views::transform([](const auto e) { return std::log2(static_cast<float>(e) + 1.0f); }) |
            std::ranges::to<std::vector>();

        ::ImGui::LabelText("average_luminance", "%0.2f", average_luminance);
        ::ImGui::PlotHistogram(
            "luminance_histogram",
            scaled_histogram.data(),
            256,
            0,
            nullptr,
            0.0f,
            std::ranges::max(scaled_histogram),
            ::ImVec2(::ImGui::GetContentRegionAvail().x, 150.0f));

        ::ImGui::PopID();
    }
    ::ImGui::End();
}

auto DebugRenderer::draw_logs() -> void
{
    if (::ImGui::Begin("Logs"))
    {
        ::ImGui::PushID("logs");

        static auto auto_scroll = true;
        static auto force_scroll_to_bottom = false;
        if (::ImGui::Checkbox("auto scroll", &auto_scroll))
        {
            if (auto_scroll)
            {
                force_scroll_to_bottom = auto_scroll;
            }
        }

        ::ImGui::BeginChild("log output");

        if (auto_scroll && !force_scroll_to_bottom)
        {
            const auto scroll_max = ::ImGui::GetScrollMaxY();
            const auto scroll_current = ::ImGui::GetScrollY();

            if (scroll_max > 0.0f && scroll_current < scroll_max)
            {
                auto_scroll = false;
            }
        }

        for (const auto &line : ufps::log::history)
        {
            switch (line[1])
            {
                case 'D': ::ImGui::TextColored({0.0f, 0.5f, 1.0f, 1.0f}, "%s", line.c_str()); break;
                case 'I': ::ImGui::TextColored({1.0f, 1.0f, 1.0f, 1.0f}, "%s", line.c_str()); break;
                case 'W': ::ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "%s", line.c_str()); break;
                case 'E': ::ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", line.c_str()); break;
                default: ::ImGui::TextColored({1.0f, 0.412f, 0.706f, 1.0f}, "%s", line.c_str()); break;
            }
        }

        if (auto_scroll)
        {
            ::ImGui::SetScrollHereY(1.0f);
        }

        ::ImGui::EndChild();

        force_scroll_to_bottom = false;

        ::ImGui::PopID();
    }

    ::ImGui::End();
}

auto DebugRenderer::draw_bloom_mips() -> void
{
    auto &tm = service<TextureManager>();

    if (::ImGui::Begin("Bloom Mips"))
    {
        ::ImGui::PushID("bloom_mips");

        static constexpr auto width = 175.0f;
        const auto aspect_ratio =
            static_cast<float>(window_.render_width()) / static_cast<float>(window_.render_height());

        for (const auto &mip : bloom_mips_)
        {
            ::ImGui::Image(
                tm.texture(mip.colour_texture_bindless_handle_0)->native_handle(),
                ::ImVec2(width * aspect_ratio, width),
                ::ImVec2(0.0f, 1.0f),
                ::ImVec2(1.0f, 0.0f));
            ::ImGui::SameLine();
        }

        ::ImGui::PopID();
    }

    ::ImGui::End();
}

auto DebugRenderer::draw_render_targets() -> void
{
    auto &tm = service<TextureManager>();

    if (::ImGui::Begin("Render Targets"))
    {
        ::ImGui::PushID("render_targets");

        static constexpr auto width = 175.0f;
        const auto aspect_ratio =
            static_cast<float>(window_.render_width()) / static_cast<float>(window_.render_height());

        ::ImGui::Image(
            tm.texture(ssao_blur_rt_.colour_texture_bindless_handle_0)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();

        ::ImGui::Image(
            tm.texture(gbuffer_rt_.colour_texture_bindless_handle_0)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();

        ::ImGui::Image(
            tm.texture(gbuffer_rt_.colour_texture_bindless_handle_1)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();

        ::ImGui::Image(
            tm.texture(gbuffer_rt_.colour_texture_bindless_handle_2)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();

        ::ImGui::Image(
            tm.texture(gbuffer_rt_.colour_texture_bindless_handle_3)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();

        ::ImGui::PopID();
    }

    ::ImGui::End();
}

auto DebugRenderer::draw_metrics() -> void
{
    if (::ImGui::Begin("Metrics"))
    {
        ::ImGui::PushID("metrics");

        static auto frame_allocations = std::vector<float>(1000u);
        frame_allocations.erase(std::ranges::begin(frame_allocations));
        frame_allocations.push_back(static_cast<float>(metrics().frame_allocated_bytes / 1024.0f));

        ::ImGui::PlotLines(
            "frame allocations",
            frame_allocations.data(),
            frame_allocations.size(),
            0,
            nullptr,
            0.0f,
            std::numeric_limits<float>::max(),
            ::ImVec2(0.0f, 80.0f));

        const auto m = metrics();

        ::ImGui::LabelText("total_allocation_count", "%zu", m.total_allocation_count);
        ::ImGui::LabelText("live_allocation_count", "%zu", m.live_allocation_count);
        ::ImGui::LabelText("total_allocated_bytes", "%zu", m.total_allocated_bytes);
        ::ImGui::LabelText("live_allocated_bytes", "%zu", m.live_allocated_bytes);
        ::ImGui::LabelText("frame_allocated_bytes", "%zu", m.frame_allocated_bytes);

        ::ImGui::PopID();
    }

    ::ImGui::End();
}

auto DebugRenderer::draw_inspector(Scene &scene) -> void
{
    if (!std::holds_alternative<std::monostate>(selected_))
    {
        ::ImGui::Begin("Inspector");

        const auto &[em, rem, ps, tm, lm] =
            services<EntityManager, RenderEntityManager, PhysicsSystem, TextureManager, LightManager>();

        if (auto *selected_entity = std::get_if<EntityHandle>(&selected_))
        {
            auto entity = em[*selected_entity];
            contract_assert(entity);

            ::ImGui::PushStyleColor(ImGuiCol_Button, ::ImVec4(0.80f, 0.15f, 0.15f, 1.00f));
            ::ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ::ImVec4(0.95f, 0.25f, 0.25f, 1.00f));
            ::ImGui::PushStyleColor(ImGuiCol_ButtonActive, ::ImVec4(0.65f, 0.10f, 0.10f, 1.00f));

            if (::ImGui::Button("Delete"))
            {
                scene.remove(*selected_entity);
                selected_ = std::monostate{};
                ::ImGui::PopStyleColor(3);
                ::ImGui::End();

                return;
            }

            ::ImGui::PopStyleColor(3);

            ::ImGui::Text("entity");
            ::ImGui::SameLine();

            static auto buffer = std::array<char, 256>{};
            const auto name = entity->name();
            ensure(std::ranges::size(name) < std::ranges::size(buffer), "{} too long", name);
            std::ranges::copy(std::ranges::begin(name), std::ranges::end(name) + 1zu, std::ranges::begin(buffer));

            ::ImGui::InputText("Name", std::ranges::data(buffer), std::ranges::size(buffer));
            ::ImGui::SameLine();
            if (::ImGui::Button("Set Name"))
            {
                entity->set_name(std::string(std::ranges::data(buffer)));
                log::debug("new name: {}", std::ranges::data(buffer));
            }

            ::ImGui::Checkbox("Enable snap", std::addressof(snap_enabled_));

            if (::ImGui::Button("Add Child Entity"))
            {
                static auto counter = 0zu;
                const auto handle = em.insert({std::format("{}_child_{}", entity->name(), counter++), {}, {}});

                entity->add_child(handle);
                scene.add(handle);
                selected_ = handle;
            }

            {
                auto to_delete = EntityHandle{};
                auto to_highlight = EntityHandle{};
                auto to_duplicate = EntityHandle{};

                for (const auto &[index, handle] : std::views::enumerate(entity->children()))
                {
                    const auto child = em[handle];
                    contract_assert(child);

                    const auto header = std::format("child_entity {}", child->name());

                    if (::ImGui::CollapsingHeader(header.c_str()))
                    {
                        ::ImGui::PushID(index);

                        ::ImGui::PushStyleColor(ImGuiCol_Button, ::ImVec4(0.80f, 0.15f, 0.15f, 1.00f));
                        ::ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ::ImVec4(0.95f, 0.25f, 0.25f, 1.00f));
                        ::ImGui::PushStyleColor(ImGuiCol_ButtonActive, ::ImVec4(0.65f, 0.10f, 0.10f, 1.00f));

                        if (::ImGui::Button("Delete"))
                        {
                            to_delete = handle;
                        }

                        ::ImGui::PopStyleColor(3);

                        ::ImGui::SameLine();

                        if (::ImGui::Button("Select"))
                        {
                            selected_ = handle;
                        }

                        ::ImGui::SameLine();

                        if (::ImGui::Button("Duplicate"))
                        {
                            to_duplicate = handle;
                        }

                        ::ImGui::PopID();
                    }

                    if (::ImGui::IsItemHovered())
                    {
                        log::debug("hover {}", index);
                        to_highlight = handle;
                    }
                }

                if (to_delete)
                {
                }

                if (to_duplicate)
                {
                    const auto child = em[to_duplicate];
                    static int counter = 0;

                    const auto name = std::format("{}_copy_{}", child->name(), counter++);
                    const auto handle = em.insert({name, child->render_entities(), child->transform()});
                    scene.add(handle);
                    entity->add_child(handle);
                    selected_ = handle;
                }

                highlight_entity_ = to_highlight;
            }

            auto groups = rem.groups() | std::ranges::to<std::vector>();
            std::ranges::sort(groups);

            if (::ImGui::BeginCombo("add render entity group", "Select one", 0))
            {
                for (const auto &name : groups)
                {
                    if (::ImGui::Selectable(name.c_str(), false))
                    {
                        entity->add_render_entities(rem[name]);
                    }
                }

                ::ImGui::EndCombo();
            }

            {
                auto to_delete = RenderEntityHandle{};
                auto to_highlight = RenderEntityHandle{};

                for (const auto &[index, handle] : std::views::enumerate(entity->render_entities()))
                {
                    auto render_entity = rem[handle];

                    const auto header = std::format("render_entity {}", index);

                    if (::ImGui::CollapsingHeader(header.c_str()))
                    {
                        ::ImGui::PushID(index);

                        if (::ImGui::Button("Delete"))
                        {
                            to_delete = handle;
                        }

                        const auto *albedo_texture = tm.texture(render_entity->albedo_texture_bindless_handle());
                        ::ImGui::Image(
                            albedo_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        const auto *normal_texture = tm.texture(render_entity->normal_texture_bindless_handle());
                        ::ImGui::SameLine();
                        ::ImGui::Image(
                            normal_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        const auto *specular_texture = tm.texture(render_entity->specular_texture_bindless_handle());
                        ::ImGui::SameLine();
                        ::ImGui::Image(
                            specular_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        const auto *ao_texture = tm.texture(render_entity->ao_texture_bindless_handle());
                        ::ImGui::Image(
                            ao_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        const auto *glossiness_texture =
                            tm.texture(render_entity->glossiness_texture_bindless_handle());
                        ::ImGui::SameLine();
                        ::ImGui::Image(
                            glossiness_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        const auto *emissive_texture = tm.texture(render_entity->emissive_texture_bindless_handle());
                        ::ImGui::SameLine();
                        ::ImGui::Image(
                            emissive_texture->native_handle(),
                            ::ImVec2(64.0f, 64.0f),
                            ::ImVec2(0.0f, 1.0f),
                            ::ImVec2(1.0f, 0.0f));

                        ::ImGui::PopID();
                    }

                    if (::ImGui::IsItemHovered())
                    {
                        to_highlight = handle;
                    }
                }

                if (to_delete)
                {
                    entity->remove_render_entity(to_delete);
                }

                highlight_render_entity_ = to_highlight;
            }

            if (::ImGui::Button("add rigid body"))
            {
                const auto body = ps.create_box({{-1.0f}, {1.0f}}, entity->transform().position, PhysicsLayer::STATIC);
                entity->add_rigid_body(body);
                selected_ = body;
            }

            {
                auto to_delete = RigidBodyHandle{};
                auto to_duplicate = RigidBodyHandle{};
                auto to_highlight = RigidBodyHandle{};

                for (const auto &[index, handle] : std::views::enumerate(entity->rigid_bodies()))
                {
                    const auto header = std::format("rigid_body {}", index);

                    if (::ImGui::CollapsingHeader(header.c_str()))
                    {
                        ::ImGui::PushID(index);

                        if (::ImGui::Button("Select"))
                        {
                            selected_ = handle;
                        }

                        ::ImGui::SameLine();

                        if (::ImGui::Button("Delete"))
                        {
                            to_delete = handle;
                        }

                        ::ImGui::SameLine();

                        if (::ImGui::Button("Duplicate"))
                        {
                            to_duplicate = handle;
                        }

                        ::ImGui::PopID();
                    }

                    if (::ImGui::IsItemHovered())
                    {
                        to_highlight = handle;
                    }
                }

                if (to_delete)
                {
                    service<PhysicsSystem>().remove_rigid_body(to_delete);
                }

                if (to_duplicate)
                {
                    const auto handle = ps.duplicate_rigid_body(to_duplicate);
                    entity->add_rigid_body(handle);
                    selected_ = handle;
                }

                highlight_rigid_body_ = to_highlight;

                {
                    auto value = entity->emissive_strength();
                    if (::ImGui::SliderFloat("emissive_strength", &value, 0.0f, 10.0f))
                    {
                        entity->set_emissive_strength(value);
                    }
                }

                const auto transforms =
                    std::array<Matrix4, 3u>{entity->transform(), entity->local_transform(), entity->parent_transform()};
                const auto transform_names = std::array<std::string, 3u>{"world", "local", "parent"};

                for (const auto &[transform, name] : std::views::zip(transforms, transform_names))
                {
                    ::ImGui::Text(name.c_str());
                    ::ImGui::SameLine();

                    ::ImGui::BeginTable(
                        name.c_str(),
                        4,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);

                    for (auto row = 0; row < 4; ++row)
                    {
                        ::ImGui::TableNextRow();
                        for (auto col = 0; col < 4; ++col)
                        {
                            ::ImGui::TableSetColumnIndex(col);
                            ::ImGui::Text("%0.2f", transform[col * 4 + row]);
                        }
                    }

                    ::ImGui::EndTable();
                }
            }
        }
        else if (auto *selected_light = std::get_if<LightHandle>(&selected_))
        {
            auto light = lm[*selected_light];
            contract_assert(light);

            ::ImGui::Text("point light");

            float pos[] = {light->position.x, light->position.y, light->position.z};
            if (::ImGui::SliderFloat3("position", pos, -100.0f, 100.0f))
            {
                light->position = {pos[0], pos[1], pos[2]};
            }

            float colour[3]{};
            std::memcpy(colour, &light->colour, sizeof(colour));

            if (::ImGui::ColorPicker3("light colour", colour))
            {
                std::memcpy(&light->colour, colour, sizeof(colour));
            }

            float atten[] = {light->constant_attenuation, light->linear_attenuation, light->quadratic_attenuation};
            if (::ImGui::SliderFloat3("attenuation", atten, 0.0f, 2.0f))
            {
                light->constant_attenuation = atten[0];
                light->linear_attenuation = atten[1];
                light->quadratic_attenuation = atten[2];
            }

            auto intensity = light->intensity;
            if (::ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
            {
                light->intensity = intensity;
            }
        }

        ::ImGui::End();
    }
}

auto DebugRenderer::draw_gizmo(const Camera &camera) -> void
{
    const auto &[em, ps, lm, cm] = services<EntityManager, PhysicsSystem, LightManager, CameraManager>();

    if (!std::holds_alternative<std::monostate>(selected_))
    {
        const auto &camera_data = camera.data();

        if (auto *selected_entity = std::get_if<EntityHandle>(&selected_))
        {
            auto entity = em[*selected_entity];
            contract_assert(entity);

            static float snap_translation[3] = {1.0f, 1.0f, 1.0f};
            auto world_matrix = Matrix4{entity->transform()};

            ::ImGuizmo::Manipulate(
                camera_data.view.data().data(),
                camera_data.projection.data().data(),
                ::ImGuizmo::TRANSLATE | ::ImGuizmo::ROTATE,
                ::ImGuizmo::WORLD,
                world_matrix.data().data(),
                nullptr,
                snap_enabled_ ? snap_translation : nullptr,
                nullptr,
                nullptr);

            if (::ImGuizmo::IsUsing())
            {
                const auto parent = Matrix4{entity->parent_transform()};
                const auto inverse_parent = Matrix4::invert(parent);
                const auto local = inverse_parent * world_matrix;

                entity->set_transform(local);
            }
        }
        else if (auto *selected_light = std::get_if<LightHandle>(&selected_))
        {
            auto light = lm[*selected_light];
            contract_assert(light);

            auto transform = Matrix4{light->position};

            ::ImGuizmo::Manipulate(
                camera_data.view.data().data(),
                camera_data.projection.data().data(),
                ::ImGuizmo::TRANSLATE | ::ImGuizmo::ROTATE,
                ::ImGuizmo::WORLD,
                transform.data().data(),
                nullptr,
                nullptr,
                nullptr,
                nullptr);

            if (::ImGuizmo::IsUsing())
            {
                const auto new_transform = Transform{transform};
                light->position = new_transform.position;
            }
        }
        else if (auto *selected_rigid_body = std::get_if<RigidBodyHandle>(&selected_))
        {
            if (auto rigid_body = ps.rigid_body(*selected_rigid_body); rigid_body)
            {
                auto &rb = *rigid_body;

                auto world_matrix = Matrix4{rb.transform()};

                ::ImGuizmo::Manipulate(
                    camera_data.view.data().data(),
                    camera_data.projection.data().data(),
                    ::ImGuizmo::TRANSLATE | ::ImGuizmo::SCALE | ::ImGuizmo::ROTATE,
                    ::ImGuizmo::WORLD,
                    world_matrix.data().data(),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr);

                if (::ImGuizmo::IsUsing())
                {
                    const auto parent = Matrix4{rb.parent_transform()};
                    const auto inverse_parent = Matrix4::invert(parent);
                    const auto local = inverse_parent * world_matrix;

                    rb.set_local_transform(local);
                }
            }
        }
        else if (auto *selected_camera = std::get_if<CameraHandle>(&selected_))
        {
            auto camera = cm[*selected_camera];
            contract_assert(camera);

            auto world_matrix = Matrix4{camera->transform()};

            ::ImGuizmo::Manipulate(
                camera_data.view.data().data(),
                camera_data.projection.data().data(),
                ::ImGuizmo::TRANSLATE | ::ImGuizmo::ROTATE,
                ::ImGuizmo::WORLD,
                world_matrix.data().data(),
                nullptr,
                nullptr,
                nullptr,
                nullptr);

            if (::ImGuizmo::IsUsing())
            {
                const auto parent = Matrix4{camera->parent_transform()};
                const auto inverse_parent = Matrix4::invert(parent);
                const auto local = inverse_parent * world_matrix;

                camera->set_transform(local);
            }
        }
    }
}
}
