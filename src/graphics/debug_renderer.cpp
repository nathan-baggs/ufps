#include "graphics/debug_renderer.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>

#include <imgui.h>

#include <ImGuizmo.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

#include "core/scene.h"
#include "events/mouse_button_event.h"
#include "graphics/colour.h"
#include "graphics/line_data.h"
#include "graphics/opengl.h"
#include "graphics/point_light.h"
#include "graphics/utils.h"
#include "graphics/window.h"
#include "maths/aabb.h"
#include "maths/matrix4.h"
#include "maths/ray.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
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

}

namespace ufps
{
DebugRenderer::DebugRenderer(
    const Window &window,
    ResourceLoader &resource_loader,
    TextureManager &texture_manager,
    MeshManager &mesh_manager)
    : Renderer{window, resource_loader, texture_manager, mesh_manager}
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
                { return create_aabb_lines(e.aabb(), selected_entity->transform(), {0.4f, 0.4f, 0.4f}); }) |
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

    const auto [vertex_buffer_handle, index_buffer_handle] = scene.mesh_manager().native_handle();
    ::glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_handle);
    ::glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        1,
        camera_buffer_.native_handle(),
        camera_buffer_.frame_offset_bytes(),
        sizeof(CameraData));
    ::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_handle);

    const auto cube_parts = scene.mesh_manager().mesh("cube");
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

    ::ImGui::Begin("scene");

    ::ImGui::LabelText("FPS", "%0.1f", io.Framerate);
    ::ImGui::LabelText("Debug Lines", "%0.1f", static_cast<float>(debug_line_count));

    if (::ImGui::Button("save"))
    {
        const auto scene_yaml = yaml::serialise(scene.description());
        auto out = std::ofstream("scene.yaml");

        out << scene_yaml;
    }

    if (::ImGui::Button("add light"))
    {
        const auto handle = scene.lights().lights.emplace(
            PointLight{
                .position = {},
                .colour = {.r = 1.0f, .g = 1.0f, .b = 1.0f},
                .constant_attenuation = 1.0f,
                .linear_attenuation = 0.007f,
                .quadratic_attenuation = 0.0002f,
                .specular_power = 32.0f,
                .intensity = 1.0f});
        selected_ = handle;
    }

    ::ImGui::Text("tone map options");

    {
        auto value = scene.tone_map_options().max_brightness;
        if (::ImGui::SliderFloat("maximum brightness", &value, 0.0f, 100.0f))
        {
            scene.tone_map_options().max_brightness = value;
        }
    }

    {
        auto value = scene.tone_map_options().contrast;
        if (::ImGui::SliderFloat("contrast", &value, 0.0f, 5.0f))
        {
            scene.tone_map_options().contrast = value;
        }
    }

    {
        auto value = scene.tone_map_options().linear_section_start;
        if (::ImGui::SliderFloat("linear section start", &value, 0.0f, 1.0f))
        {
            scene.tone_map_options().linear_section_start = value;
        }
    }

    {
        auto value = scene.tone_map_options().linear_section_length;
        if (::ImGui::SliderFloat("linear section length", &value, 0.0f, 1.0f))
        {
            scene.tone_map_options().linear_section_length = value;
        }
    }

    {
        auto value = scene.tone_map_options().black_tightness;
        if (::ImGui::SliderFloat("black tightness", &value, 0.0f, 3.0f))
        {
            scene.tone_map_options().black_tightness = value;
        }
    }

    {
        auto value = scene.tone_map_options().pedestal;
        if (::ImGui::SliderFloat("pedestal", &value, 0.0f, 1.0f))
        {
            scene.tone_map_options().pedestal = value;
        }
    }

    {
        auto value = scene.tone_map_options().gamma;
        if (::ImGui::SliderFloat("gamma", &value, 0.0f, 5.0f))
        {
            scene.tone_map_options().gamma = value;
        }
    }

    ::ImGui::Text("ssao options");

    {
        auto value = scene.ssao_options().enabled;
        if (::ImGui::Checkbox("enabled", &value))
        {
            scene.ssao_options().enabled = value;
        }
    }

    {
        auto value = static_cast<int>(scene.ssao_options().sample_count);
        if (::ImGui::SliderInt("sample_count", &value, 1, 64))
        {
            scene.ssao_options().sample_count = value;
        }
    }

    {
        auto value = scene.ssao_options().radius;
        if (::ImGui::SliderFloat("radius", &value, 0.1f, 2.0f))
        {
            scene.ssao_options().radius = value;
        }
    }

    {
        auto value = scene.ssao_options().bias;
        if (::ImGui::SliderFloat("bias", &value, 0.01f, 0.1f))
        {
            scene.ssao_options().bias = value;
        }
    }

    {
        auto value = scene.ssao_options().power;
        if (::ImGui::SliderFloat("power", &value, 1.0f, 4.0f))
        {
            scene.ssao_options().power = value;
        }
    }

    ::ImGui::Text("exposure options");

    {
        auto value = scene.exposure_options().min_log_luminance;
        if (::ImGui::SliderFloat("min_log_luminance", &value, -10.0f, 10.0f))
        {
            scene.exposure_options().min_log_luminance = value;
        }
    }

    {
        auto value = scene.exposure_options().max_log_luminance;
        if (::ImGui::SliderFloat("max_log_luminance", &value, -10.0f, 10.0f))
        {
            scene.exposure_options().max_log_luminance = value;
        }
    }

    auto average_luminance = 0.0f;
    ::glGetNamedBufferSubData(
        average_luminance_buffer_.native_handle(), 0, sizeof(average_luminance), &average_luminance);

    ::ImGui::LabelText("average luminance", "%f", average_luminance);

    std::uint32_t histogram[256]{};
    ::glGetNamedBufferSubData(luminance_histogram_buffer_.native_handle(), 0, sizeof(histogram), &histogram);

    const auto scaled_histogram =
        histogram | std::views::transform([](const auto e) { return std::log2(static_cast<float>(e) + 1.0f); }) |
        std::ranges::to<std::vector>();

    ::ImGui::PlotHistogram(
        "luminance",
        scaled_histogram.data(),
        256,
        0,
        nullptr,
        0.0f,
        std::ranges::max(scaled_histogram),
        ::ImVec2(::ImGui::GetContentRegionAvail().x, 150.0f));

    auto mesh_names = scene.mesh_manager().mesh_names();
    std::ranges::sort(mesh_names);
    const auto mesh_names_cstr = mesh_names |                                                     //
                                 std::views::filter([](const auto &e) { return !e.empty(); }) |   //
                                 std::views::transform([](const auto &e) { return e.c_str(); }) | //
                                 std::ranges::to<std::vector>();

    auto mesh_selected_index = std::optional<std::uint32_t>{};

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
        scene.create_entity(mesh_names_cstr[*mesh_selected_index]);
        selected_ = &scene.entities().back();
    }

    if (::ImGui::Button("delete"))
    {
        if (auto **selected_entity = std::get_if<Entity *>(&selected_))
        {
            auto *entity = *selected_entity;
            scene.remove(*entity);
            selected_ = std::monostate{};
        }
        if (auto *selected_entity = std::get_if<PointLightHandle>(&selected_))
        {
            scene.lights().lights.remove(*selected_entity);
            selected_ = std::monostate{};
        }
    }

    ::ImGui::SameLine();

    if (::ImGui::Button("duplicate"))
    {
        if (auto **selected_entity = std::get_if<Entity *>(&selected_))
        {
            auto *entity = *selected_entity;
            auto *new_entity = scene.create_entity(entity->name());
            new_entity->set_transform(entity->transform());
            selected_ = new_entity;
        }
        else if (auto *selected_light = std::get_if<PointLightHandle>(&selected_))
        {
            const auto light = scene.lights().lights[*selected_light];
            ensure(!!light, "missing light?");

            selected_ = scene.lights().lights.emplace(*light);
        }
    }

    for (auto &entity : scene.entities())
    {
        ::ImGui::CollapsingHeader(entity.name().c_str());
    }

    for (const auto &[index, light] : std::views::enumerate(scene.lights().lights.handles()))
    {
        const auto light_name = std::format("light {}", index);

        ::ImGui::CollapsingHeader(light_name.c_str());
    }

    float amb_colour[3]{};
    std::memcpy(amb_colour, &scene.lights().ambient, sizeof(amb_colour));

    if (::ImGui::ColorPicker3("ambient light colour", amb_colour))
    {
        std::memcpy(&scene.lights().ambient, amb_colour, sizeof(amb_colour));
    }
    const auto camera_transform = scene.camera().data().view;

    ::ImGui::BeginTable(
        "transform", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);

    for (auto row = 0; row < 4; ++row)
    {
        ::ImGui::TableNextRow();
        for (auto col = 0; col < 4; ++col)
        {
            ::ImGui::TableSetColumnIndex(col);
            ::ImGui::Text("%0.2f", camera_transform[col * 4 + row]);
        }
    }

    ::ImGui::EndTable();

    ::ImGui::End();
    ::ImGui::Begin("log");

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

    for (const auto &line : log::history)
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

    ::ImGui::End();

    ::ImGui::Begin("render_targets");
    static constexpr auto width = 175.0f;
    const auto aspect_ratio = static_cast<float>(window_.render_width()) / static_cast<float>(window_.render_height());

    ::ImGui::Image(
        scene.texture_manager().texture(ssao_blur_rt_.first_colour_attachment_index)->native_handle(),
        ::ImVec2(width * aspect_ratio, width),
        ::ImVec2(0.0f, 1.0f),
        ::ImVec2(1.0f, 0.0f));
    ::ImGui::SameLine();

    for (auto i = 0u; i < gbuffer_rt_.colour_attachment_count; ++i)
    {
        const auto tex = scene.texture_manager().texture(gbuffer_rt_.first_colour_attachment_index + i);
        ::ImGui::Image(
            tex->native_handle(), ::ImVec2(width * aspect_ratio, width), ::ImVec2(0.0f, 1.0f), ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();
    }

    ::ImGui::End();

    if (!std::holds_alternative<std::monostate>(selected_))
    {
        ::ImGui::Begin("inspector");

        if (auto **selected_entity = std::get_if<Entity *>(&selected_))
        {
            auto *entity = *selected_entity;
            ::ImGui::Text("entity: %s", entity->name().c_str());

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
                const auto material_index = render_entity.material_index();
                const auto &material = scene.material_manager().material(material_index);

                const auto *albedo_texture = scene.texture_manager().texture(material.albedo_texture_index);
                if (albedo_texture)
                {
                    ::ImGui::Image(
                        albedo_texture->native_handle(),
                        ::ImVec2(64.0f, 64.0f),
                        ::ImVec2(0.0f, 1.0f),
                        ::ImVec2(1.0f, 0.0f));
                }

                const auto *normal_texture = scene.texture_manager().texture(material.normal_texture_index);
                if (normal_texture)
                {
                    ::ImGui::SameLine();
                    ::ImGui::Image(
                        normal_texture->native_handle(),
                        ::ImVec2(64.0f, 64.0f),
                        ::ImVec2(0.0f, 1.0f),
                        ::ImVec2(1.0f, 0.0f));
                }

                const auto *specular_texture = scene.texture_manager().texture(material.specular_texture_index);
                if (specular_texture)
                {
                    ::ImGui::SameLine();
                    ::ImGui::Image(
                        specular_texture->native_handle(),
                        ::ImVec2(64.0f, 64.0f),
                        ::ImVec2(0.0f, 1.0f),
                        ::ImVec2(1.0f, 0.0f));
                }
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

            ::ImGui::SliderFloat("power", &light->specular_power, 0.0f, 100.0f);

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
