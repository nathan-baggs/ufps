#include "graphics/debug_renderer.h"

#include <cstring>
#include <format>
#include <locale>
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
#include "graphics/utils.h"
#include "graphics/window.h"
#include "maths/aabb.h"
#include "maths/matrix4.h"
#include "maths/ray.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
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
    if (std::holds_alternative<const Entity *>(selected_))
    {
        const auto *selected_entity = std::get<const Entity *>(selected_);
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

    const auto light_transform = Transform{scene.lights().light.position, {debug_light_scale}, {}};
    const auto light_model = Matrix4{light_transform};

    const auto debug_light_aabb = ufps::AABB{
        .min = light_model * Vector4{-1.0f, -1.0f, -1.0f, 1.0f},
        .max = light_model * Vector4{1.0f},
    };
    debug_lines_.append_range(create_aabb_lines(debug_light_aabb, {}, {1.0f, 0.0f, 0.0f}));

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

    debug_light_program_.use();

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

    ::glProgramUniformMatrix4fv(debug_light_program_.native_handle(), 0u, 1u, GL_FALSE, light_model.data().data());
    ::glProgramUniform3f(
        debug_light_program_.native_handle(),
        1u,
        scene.lights().light.colour.r,
        scene.lights().light.colour.g,
        scene.lights().light.colour.b);

    ::glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, reinterpret_cast<const void *>(cube_indices_offset_bytes));

    auto debug_line_count = 0zu;

    if (!debug_lines_.empty())
    {
        debug_line_program_.use();
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

    const auto mesh_names_cstr = mesh_manager_.mesh_names() |
                                 std::views::transform([](const auto &e) { return e.c_str(); }) |
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
    }

    for (auto &entity : scene.entities())
    {
        ::ImGui::CollapsingHeader(entity.name().c_str());

        if (const auto selected_entity = std::get_if<const Entity *>(&selected_);
            selected_entity && *selected_entity == &entity)
        {
            auto transform = Matrix4{entity.transform()};
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

            entity.set_transform(transform);
        }
    }

    if (::ImGui::CollapsingHeader("lights"))
    {
        float amb_colour[3]{};
        std::memcpy(amb_colour, &scene.lights().ambient, sizeof(amb_colour));

        if (::ImGui::ColorPicker3("ambient light colour", amb_colour))
        {
            std::memcpy(&scene.lights().ambient, amb_colour, sizeof(amb_colour));
        }

        float pos[] = {
            scene.lights().light.position.x, scene.lights().light.position.y, scene.lights().light.position.z};
        if (::ImGui::SliderFloat3("position", pos, -100.0f, 100.0f))
        {
            scene.lights().light.position = {pos[0], pos[1], pos[2]};
        }

        float colour[3]{};
        std::memcpy(colour, &scene.lights().light.colour, sizeof(colour));

        if (::ImGui::ColorPicker3("light colour", colour))
        {
            std::memcpy(&scene.lights().light.colour, colour, sizeof(colour));
        }

        ::ImGui::SliderFloat("power", &scene.lights().light.specular_power, 0.0f, 100.0f);

        float atten[] = {
            scene.lights().light.constant_attenuation,
            scene.lights().light.linear_attenuation,
            scene.lights().light.quadratic_attenuation};
        if (::ImGui::SliderFloat3("attenuation", atten, 0.0f, 2.0f))
        {
            scene.lights().light.constant_attenuation = atten[0];
            scene.lights().light.linear_attenuation = atten[1];
            scene.lights().light.quadratic_attenuation = atten[2];
        }
    }

    if (const auto selected_light = std::get_if<const LightData *>(&selected_); selected_light)
    {
        auto transform = Matrix4{scene.lights().light.position};
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
        scene.lights().light.position = new_transform.position;
    }

    ::ImGui::End();
    ::ImGui::Begin("log");

    ::ImGui::BeginChild("log output");
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
    ::ImGui::EndChild();

    ::ImGui::End();

    ::ImGui::Begin("render_targets");
    static constexpr auto width = 175.0f;
    const auto aspect_ratio = static_cast<float>(window_.render_width()) / static_cast<float>(window_.render_height());
    for (auto i = 0u; i < gbuffer_rt_.colour_attachment_count; ++i)
    {
        const auto tex = scene.texture_manager().texture(gbuffer_rt_.first_colour_attachment_index + i);
        ::ImGui::Image(
            tex->native_handle(), ::ImVec2(width * aspect_ratio, width), ::ImVec2(0.0f, 1.0f), ::ImVec2(1.0f, 0.0f));
        ::ImGui::SameLine();
    }
    ::ImGui::Image(
        scene.texture_manager().texture(gbuffer_rt_.depth_attachment_index)->native_handle(),
        ::ImVec2(width * aspect_ratio, width),
        ::ImVec2(0.0f, 1.0f),
        ::ImVec2(1.0f, 0.0f));
    ::ImGui::End();

    ::ImGui::Render();
    ::ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

    if (click_)
    {
        const auto pick_ray = screen_ray(*click_, window_, scene.camera());
        const auto intersection = scene.intersect_ray(pick_ray);
        if (intersection)
        {
            selected_ = intersection->entity;
        }
        else
        {
            selected_ = std::monostate{};
        }

        if (const auto light_intersection = intersect(pick_ray, debug_light_aabb); light_intersection)
        {
            if (!intersection || light_intersection < intersection->distance)
            {
                selected_ = std::addressof(scene.lights());
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
