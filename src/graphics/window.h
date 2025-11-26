#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include <windows.h>

#include "events/event.h"
#include "utils/auto_release.h"

namespace ufps
{

enum class WindowMode
{
    FULLSCREEN,
    WINDOWED
};

/**
 * Represents an actual windows window.
 */
class Window
{
  public:
    Window(
        WindowMode mode,
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t x,
        std::uint32_t y,
        bool mouse_locked = false);
    ~Window() = default;

    Window(const Window &) = delete;
    auto operator=(const Window &) -> Window & = delete;

    Window(Window &&) = default;
    auto operator=(Window &&) -> Window & = default;

    auto pump_event() const -> std::optional<Event>;
    auto swap() const -> void;
    auto native_handle() const -> HWND;
    auto render_width() const -> std::uint32_t;
    auto render_height() const -> std::uint32_t;
    auto window_width() const -> std::uint32_t;
    auto window_height() const -> std::uint32_t;
    auto set_title(const std::string &title) const -> void;
    auto mode() const -> WindowMode;
    auto set_mode(WindowMode mode) -> void;

  private:
    AutoRelease<::HWND, nullptr> window_;
    AutoRelease<::HDC> dc_;
    ::WNDCLASSA wc_;
    std::uint32_t width_;
    std::uint32_t height_;
    WindowMode mode_;
    bool mouse_locked_;
};

inline auto to_string(WindowMode mode) -> std::string
{
    switch (mode)
    {
        using enum WindowMode;
        case FULLSCREEN: return "FULLSCREEN";
        case WINDOWED: return "WINDOWED";
        default: return "<UNKNOWN>";
    }
}

}
