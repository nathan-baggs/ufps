#include "graphics/debug_renderer.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <meta>
#include <optional>
#include <ranges>
#include <string>

#include <imgui.h>

#include <ImGuizmo.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

#include "core/scene.h"
#include "core/service_locator.h"
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

struct SaveSceneButton
{
    ufps::Scene &scene;
};

struct AddLightButton
{
    ufps::Scene &scene;
    std::variant<std::monostate, ufps::Entity *, ufps::PointLightHandle> *selected;
};

struct Histogram
{
    std::vector<float> values;
};

struct AddEntity
{
    ufps::Scene &scene;
    std::variant<std::monostate, ufps::Entity *, ufps::PointLightHandle> *selected;
};

struct DuplicateEntity
{
    ufps::Scene &scene;
    std::variant<std::monostate, ufps::Entity *, ufps::PointLightHandle> *selected;
};

struct DeleteEntity
{
    ufps::Scene &scene;
    std::variant<std::monostate, ufps::Entity *, ufps::PointLightHandle> *selected;
};

struct Plot
{
    std::vector<float> values;
};

struct TextureController
{
    std::uint32_t handle;
    float width;
    float height;
};

template <class T>
struct Wrapper
{
    T &controller;
};

struct SameLine
{
};

struct LogView
{
};

constexpr auto clean_name(std::string_view name) -> std::string
{
    return std::string{name.substr(name.find_last_of(":") + 1)};
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

auto create_debug_controller(const std::string &label, bool &value) -> void
{
    ::ImGui::Checkbox(label.c_str(), &value);
}

auto create_debug_controller(const std::string &label, float &value) -> void
{
    ::ImGui::LabelText(label.c_str(), "%0.2f", value);
}

auto create_debug_controller(const std::string &label, std::size_t &value) -> void
{
    ::ImGui::LabelText(label.c_str(), "%zu", value);
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

auto create_debug_controller(const std::string &, SaveSceneButton &value) -> void
{
    if (::ImGui::Button("save"))
    {
        const auto scene_yaml = ufps::yaml::serialise(value.scene.description());
        ufps::ensure(scene_yaml);
        auto out = std::ofstream("scene.yaml");

        out << *scene_yaml;
    }
}

auto create_debug_controller(const std::string &, AddLightButton &value) -> void
{
    if (::ImGui::Button("add light"))
    {
        const auto handle = value.scene.lights().lights.emplace(
            ufps::PointLight{
                .position = {},
                .colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f},
                .constant_attenuation = 1.0f,
                .linear_attenuation = 0.007f,
                .quadratic_attenuation = 0.0002f,
                .intensity = 1.0f});
        *value.selected = handle;
    }
}

auto create_debug_controller(const std::string &label, Histogram &value) -> void
{
    ::ImGui::PlotHistogram(
        label.c_str(),
        value.values.data(),
        256,
        0,
        nullptr,
        0.0f,
        std::ranges::max(value.values),
        ::ImVec2(::ImGui::GetContentRegionAvail().x, 150.0f));
}

auto create_debug_controller(const std::string &, AddEntity &value) -> void
{
    auto mesh_selected_index = std::optional<std::uint32_t>{};

    auto mesh_names = ufps::service<ufps::MeshManager>().mesh_names();
    std::ranges::sort(mesh_names);
    const auto mesh_names_cstr = mesh_names |                                                     //
                                 std::views::filter([](const auto &e) { return !e.empty(); }) |   //
                                 std::views::transform([](const auto &e) { return e.c_str(); }) | //
                                 std::ranges::to<std::vector>();

    if (::ImGui::BeginCombo("mesh_names", mesh_names_cstr.front(), 0))
    {
        for (const auto &[index, name] : std::views::enumerate(mesh_names_cstr))
        {
            if (::ImGui::Selectable(name))
            {
                mesh_selected_index = index;
            }
        }
        ::ImGui::EndCombo();
    }

    if (mesh_selected_index)
    {
        value.scene.create_entity(mesh_names_cstr[*mesh_selected_index]);
        *value.selected = &value.scene.entities().back();
    }
}

auto create_debug_controller(const std::string &, DeleteEntity &value) -> void
{
    if (::ImGui::Button("delete"))
    {
        if (auto **selected_entity = std::get_if<ufps::Entity *>(value.selected))
        {
            auto *entity = *selected_entity;
            value.scene.remove(*entity);
            *value.selected = std::monostate{};
        }
        if (auto *selected_entity = std::get_if<ufps::PointLightHandle>(value.selected))
        {
            value.scene.lights().lights.remove(*selected_entity);
            *value.selected = std::monostate{};
        }
    }
}

auto create_debug_controller(const std::string &, ufps::Matrix4 &value) -> void
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

auto create_debug_controller(const std::string &, DuplicateEntity &value) -> void
{
    if (::ImGui::Button("duplicate"))
    {
        if (auto **selected_entity = std::get_if<ufps::Entity *>(value.selected))
        {
            auto *entity = *selected_entity;
            auto *new_entity = value.scene.create_entity(entity->name());
            new_entity->set_transform(entity->transform());
            *value.selected = new_entity;
        }
        else if (auto *selected_light = std::get_if<ufps::PointLightHandle>(value.selected))
        {
            const auto light = value.scene.lights().lights[*selected_light];
            ufps::ensure(!!light, "missing light?");

            *value.selected = value.scene.lights().lights.emplace(*light);
        }
    }
}
auto create_debug_controller(const std::string &, LogView &) -> void
{
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
}

auto create_debug_controller(const std::string &, Plot &value) -> void
{
    ::ImGui::PlotLines(
        "frame allocations",
        value.values.data(),
        value.values.size(),
        0,
        nullptr,
        0.0f,
        std::numeric_limits<float>::max(),
        ::ImVec2(0.0f, 80.0f));
}

auto create_debug_controller(const std::string &, SameLine &) -> void
{
    ::ImGui::SameLine();
}

auto create_debug_controller(const std::string &, TextureController &value) -> void
{
    ::ImGui::Image(value.handle, ::ImVec2(value.width, value.height), ::ImVec2(0.0f, 1.0f), ::ImVec2(1.0f, 0.0f));
}

template <class T>
auto create_debug_controls(T &&data) -> void
{
    const auto title = std::format("{}", clean_name(std::meta::display_string_of(std::meta::remove_cvref(^^T))));

    ::ImGui::PushID(title.c_str());

    ::ImGui::Text(title.c_str());

    constexpr auto ctx = std::meta::access_context::current();

    template for (constexpr auto &member :
                  std::define_static_array(std::meta::nonstatic_data_members_of(std::meta::remove_cvref(^^T), ctx)))
    {
        const auto label = clean_name(std::meta::display_string_of(member));
        create_debug_controller(label, data.[:member:]);
    }

    ::ImGui::PopID();
}

template <class... Controllers>
auto create_debug_window(const std::string &name, Controllers &&...controllers)
{
    ::ImGui::Begin(name.c_str());

    (create_debug_controls(controllers), ...);

    ::ImGui::End();
}

}

namespace ufps
{
DebugRenderer::DebugRenderer(const Window &window, ResourceLoader &resource_loader)
    : Renderer{window, resource_loader}
    , enabled_{false}
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

auto DebugRenderer::post_render(Scene &scene) -> void
{
    if (std::holds_alternative<Entity *>(selected_))
    {
        const auto *selected_entity = std::get<Entity *>(selected_);
        auto aabb_lines =
            selected_entity->render_entities() |
            std::views::transform(
                [&](const auto &e)
                { return create_aabb_lines(e.aabb(), selected_entity->transform(), {0.0f, 0.2f, 0.0f}); }) |
            std::views::join;

        debug_lines_.append_range(aabb_lines);
        debug_lines_.append_range(
            create_aabb_lines(selected_entity->aabb(), selected_entity->transform(), {0.0f, 1.0f, 0.0f}));
    }

    Renderer::post_render(scene);

    if (!enabled_)
    {
        return;
    }

    auto &texture_manager = service<TextureManager>();

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

    const auto [vertex_buffer_handle, index_buffer_handle] = service<MeshManager>().native_handle();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        1,
        camera_buffer_.native_handle(),
        camera_buffer_.frame_offset_bytes(),
        sizeof(CameraData));
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);

    const auto cube_parts = service<MeshManager>().mesh("cube");
    ensure(cube_parts.size() == 1u, "cube mesh should have exactly 1 part");
    const auto cube_indices_offset_bytes = cube_parts.front().index_offset * sizeof(std::uint32_t);
    const auto cube_vertex_offset = cube_parts.front().vertex_offset;

    for (const auto &light : scene.lights().lights.data())
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

    debug_light_program_.unbind();

    auto &&physics_debug_renderer = service<PhysicsSystem>().debug_renderer();
    if (physics_debug_renderer)
    {
        debug_lines_.append_range(physics_debug_renderer->yield_lines());
    }

    auto debug_line_count = 0zu;

    if (!debug_lines_.empty())
    {
        debug_line_program_.bind();
        debug_line_count = debug_lines_.size();

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

    struct BasicSceneInfo
    {
        float fps;
        float debug_lines;
        SaveSceneButton save_scene;
        AddLightButton add_light;
    };

    auto average_luminance = 0.0f;
    ::glGetNamedBufferSubData(
        average_luminance_buffer_.native_handle(), 0, sizeof(average_luminance), &average_luminance);

    std::uint32_t histogram[256]{};
    ::glGetNamedBufferSubData(luminance_histogram_buffer_.native_handle(), 0, sizeof(histogram), &histogram);

    auto scaled_histogram =
        histogram | std::views::transform([](const auto e) { return std::log2(static_cast<float>(e) + 1.0f); }) |
        std::ranges::to<std::vector>();

    struct Luminance
    {
        float average_luminance;
        Histogram luminance;
    };

    struct SceneControls
    {
        AddEntity add_entity;
        DeleteEntity delete_entity;
        SameLine same_line{};
        DuplicateEntity duplicate_entity;
    };

    struct RemainingSceneInfo
    {
        Colour &ambient;
        Matrix4 camera_view;
    };

    create_debug_window(
        "scene",
        BasicSceneInfo{
            .fps = io.Framerate,
            .debug_lines = static_cast<float>(debug_line_count),
            .save_scene = {.scene = scene},
            .add_light = {.scene = scene, .selected = &selected_}},
        scene.tone_map_options(),
        scene.ssao_options(),
        scene.bloom_options(),
        scene.fog_options(),
        scene.chromatic_aberration_options(),
        scene.vignette_options(),
        scene.film_grain_options(),
        scene.exposure_options(),
        Luminance{
            .average_luminance = average_luminance, .luminance = Histogram{.values = std::move(scaled_histogram)}},
        SceneControls{
            .add_entity = {.scene = scene, .selected = &selected_},
            .delete_entity = {.scene = scene, .selected = &selected_},
            .same_line = {},
            .duplicate_entity = {.scene = scene, .selected = &selected_}},
        RemainingSceneInfo{.ambient = scene.lights().ambient, .camera_view = scene.camera().data().view});

    struct LogWindow
    {
        LogView view;
    };
    create_debug_window("logs", LogWindow{});

    ::ImGui::Begin("bloom_mips");

    static constexpr auto width = 175.0f;
    const auto aspect_ratio = static_cast<float>(window_.render_width()) / static_cast<float>(window_.render_height());

    for (const auto &mip : bloom_mips_)
    {
        ::ImGui::Image(
            texture_manager.texture(mip.colour_texture_bindless_handle_0)->native_handle(),
            ::ImVec2(width * aspect_ratio, width),
            ::ImVec2(0.0f, 1.0f),
            ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();
    }

    ::ImGui::End();

    static auto frame_allocations = Plot{.values = std::vector<float>(1000u)};
    frame_allocations.values.erase(std::ranges::begin(frame_allocations.values));
    frame_allocations.values.push_back(static_cast<float>(metrics().frame_allocated_bytes / 1024.0f));

    create_debug_window("metrics", metrics(), Wrapper<Plot>{.controller = frame_allocations});

    struct RenderTargets
    {
        TextureController ssao;
        SameLine same_line_0;
        TextureController gbuffer_0;
        SameLine same_line_1;
        TextureController gbuffer_1;
        SameLine same_line_2;
        TextureController gbuffer_2;
        SameLine same_line_3;
        TextureController gbuffer_3;
    };

    create_debug_window(
        "render_targets",
        RenderTargets{
            .ssao =
                {texture_manager.texture(ssao_blur_rt_.colour_texture_bindless_handle_0)->native_handle(),
                 width * aspect_ratio,
                 width},
            .same_line_0 = {},
            .gbuffer_0 =
                {texture_manager.texture(gbuffer_rt_.colour_texture_bindless_handle_0)->native_handle(),
                 width * aspect_ratio,
                 width},
            .same_line_1 = {},
            .gbuffer_1 =
                {texture_manager.texture(gbuffer_rt_.colour_texture_bindless_handle_1)->native_handle(),
                 width * aspect_ratio,
                 width},
            .same_line_2 = {},
            .gbuffer_2 =
                {texture_manager.texture(gbuffer_rt_.colour_texture_bindless_handle_2)->native_handle(),
                 width * aspect_ratio,
                 width},
            .same_line_3 = {},
            .gbuffer_3 = {
                texture_manager.texture(gbuffer_rt_.colour_texture_bindless_handle_3)->native_handle(),
                width * aspect_ratio,
                width}});

    if (!std::holds_alternative<std::monostate>(selected_))
    {
        ::ImGui::Begin("inspector");

        if (auto **selected_entity = std::get_if<Entity *>(&selected_))
        {
            auto *entity = *selected_entity;
            ::ImGui::Text("entity: %s", entity->name().c_str());

            if (::ImGui::Button("add rigid body"))
            {
                const auto body = service<PhysicsSystem>().create_box(
                    {{-1.0f}, {1.0f}}, entity->transform().position, PhysicsLayer::STATIC);
                entity->add_rigid_body(body);
            }

            {
                auto value = entity->emissive_strength();
                if (::ImGui::SliderFloat("emissive_strength", &value, 0.0f, 10.0f))
                {
                    entity->set_emissive_strength(value);
                }
            }

            auto transform = Matrix4{entity->transform()};

            ::ImGui::BeginTable(
                "transform", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);

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

            for (const auto &render_entity : entity->render_entities())
            {
                const auto *albedo_texture = texture_manager.texture(render_entity.albedo_texture_bindless_handle());
                ::ImGui::Image(
                    albedo_texture->native_handle(),
                    ::ImVec2(64.0f, 64.0f),
                    ::ImVec2(0.0f, 1.0f),
                    ::ImVec2(1.0f, 0.0f));

                const auto *normal_texture = texture_manager.texture(render_entity.normal_texture_bindless_handle());
                ::ImGui::SameLine();
                ::ImGui::Image(
                    normal_texture->native_handle(),
                    ::ImVec2(64.0f, 64.0f),
                    ::ImVec2(0.0f, 1.0f),
                    ::ImVec2(1.0f, 0.0f));

                const auto *specular_texture =
                    texture_manager.texture(render_entity.specular_texture_bindless_handle());
                ::ImGui::SameLine();
                ::ImGui::Image(
                    specular_texture->native_handle(),
                    ::ImVec2(64.0f, 64.0f),
                    ::ImVec2(0.0f, 1.0f),
                    ::ImVec2(1.0f, 0.0f));

                const auto *ao_texture = texture_manager.texture(render_entity.ao_texture_bindless_handle());
                ::ImGui::Image(
                    ao_texture->native_handle(), ::ImVec2(64.0f, 64.0f), ::ImVec2(0.0f, 1.0f), ::ImVec2(1.0f, 0.0f));

                const auto *glossiness_texture =
                    texture_manager.texture(render_entity.glossiness_texture_bindless_handle());
                ::ImGui::SameLine();
                ::ImGui::Image(
                    glossiness_texture->native_handle(),
                    ::ImVec2(64.0f, 64.0f),
                    ::ImVec2(0.0f, 1.0f),
                    ::ImVec2(1.0f, 0.0f));

                const auto *emissive_texture =
                    texture_manager.texture(render_entity.emissive_texture_bindless_handle());
                ::ImGui::SameLine();
                ::ImGui::Image(
                    emissive_texture->native_handle(),
                    ::ImVec2(64.0f, 64.0f),
                    ::ImVec2(0.0f, 1.0f),
                    ::ImVec2(1.0f, 0.0f));
            }

            const auto &camera_data = scene.camera().data();

            static float snap_translation[3] = {1.0f, 1.0f, 1.0f};

            ::ImGuizmo::Manipulate(
                camera_data.view.data().data(),
                camera_data.projection.data().data(),
                ::ImGuizmo::TRANSLATE | ::ImGuizmo::SCALE | ::ImGuizmo::ROTATE,
                ::ImGuizmo::WORLD,
                const_cast<float *>(transform.data().data()),
                nullptr,
                snap_translation,
                nullptr,
                nullptr);

            entity->set_transform(transform);
        }
        else if (auto *selected_light = std::get_if<PointLightHandle>(&selected_))
        {
            auto light = scene.lights().lights[*selected_light];
            ensure(!!light, "missing light?");

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

            auto transform = Matrix4{light->position};
            const auto &camera_data = scene.camera().data();

            ::ImGuizmo::Manipulate(
                camera_data.view.data().data(),
                camera_data.projection.data().data(),
                ::ImGuizmo::TRANSLATE | ::ImGuizmo::SCALE | ::ImGuizmo::BOUNDS | ::ImGuizmo::ROTATE,
                ::ImGuizmo::WORLD,
                const_cast<float *>(transform.data().data()),
                nullptr,
                nullptr,
                nullptr,
                nullptr);

            const auto new_transform = Transform{transform};
            light->position = new_transform.position;
        }

        ::ImGui::End();
    }

    ::ImGui::Render();
    ::ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

    if (click_)
    {
        const auto pick_ray = screen_ray(*click_, window_, scene.camera());
        auto intersection = scene.intersect_ray(pick_ray);
        if (intersection)
        {
            selected_ = intersection->entity;
        }
        else
        {
            selected_ = std::monostate{};
        }

        for (auto light_handle : scene.lights().lights.handles())
        {
            const auto light = scene.lights().lights[light_handle];

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

auto DebugRenderer::set_enabled(bool enabled) -> void
{
    enabled_ = enabled;
}
}
