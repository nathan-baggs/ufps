#include "graphics/debug_ui.h"

#include <cstring>
#include <format>
#include <optional>
#include <string>

#include <imgui.h>

#include <ImGuizmo.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

#include "events/mouse_button_event.h"
#include "graphics/opengl.h"
#include "graphics/scene.h"
#include "graphics/window.h"
#include "maths/matrix4.h"
#include "third_party/opengl/glext.h"
#include "utils/log.h"

namespace ufps
{
DebugUI::DebugUI(const Window &window)
    : window_{window}
{
    IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    auto &io = ::ImGui::GetIO();

    io.ConfigFlags |= ::ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ::ImGuiConfigFlags_NavEnableGamepad;
    ::ShowCursor(true);
    io.MouseDrawCursor = io.WantCaptureMouse;

    ::ImGui::StyleColorsDark();

    ::ImGui_ImplWin32_InitForOpenGL(window.native_handle());
    ::ImGui_ImplOpenGL3_Init();
}

DebugUI::~DebugUI()
{
    ::ImGui_ImplOpenGL3_Shutdown();
    ::ImGui_ImplWin32_Shutdown();
    ::ImGui::DestroyContext();
}

auto DebugUI::render(Scene &scene) -> void
{
    auto &io = ::ImGui::GetIO();

    ::ImGui_ImplOpenGL3_NewFrame();
    ::ImGui_ImplWin32_NewFrame();
    ::ImGui::NewFrame();

    ::ImGuizmo::SetOrthographic(false);
    ::ImGuizmo::BeginFrame();
    ::ImGuizmo::Enable(true);
    ::ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    ::ImGui::LabelText("FPS", "%0.1f", io.Framerate);

    for (auto &entity : scene.entities)
    {
        auto &material = scene.material_manager[entity.material_key];

        if (::ImGui::CollapsingHeader(entity.name.c_str()))
        {
            float colour[3]{};
            std::memcpy(colour, &material.colour, sizeof(colour));

            const auto label = std::format("{} colour", entity.name);

            if (::ImGui::ColorPicker3(label.c_str(), colour))
            {
                std::memcpy(&material.colour, colour, sizeof(colour));
            }

            auto transform = Matrix4{entity.transform};
            const auto &camera_data = scene.camera.data();

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

            entity.transform = Transform{transform};
        }
    }

    ::ImGui::Render();
    ::ImGui_ImplOpenGL3_RenderDrawData(::ImGui::GetDrawData());

    if (click_)
    {
        std::uint8_t buffer[4]{};

        ::glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        ::glReadBuffer(GL_BACK);
        ::glReadPixels(
            static_cast<::GLint>(click_->x()),
            static_cast<::GLint>(click_->y()),
            1,
            1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            buffer);

        log::debug("r: {:x} g: {:x} b: {:x}", buffer[0], buffer[1], buffer[2]);
        click_.reset();
    }
}

auto DebugUI::add_mouse_event(const MouseButtonEvent &evt) -> void
{
    auto &io = ::ImGui::GetIO();
    io.AddMouseButtonEvent(0, evt.state() == MouseButtonState::DOWN);

    click_ = evt;
}
}
