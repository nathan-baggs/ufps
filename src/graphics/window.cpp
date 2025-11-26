#include "window.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <queue>
#include <ranges>

#include "graphics/opengl.h"

#include <hidusage.h>
#include <windowsx.h>

#include "config.h"
#include "events/event.h"
#include "events/key.h"
#include "events/key_event.h"
#include "events/mouse_button_event.h"
#include "events/mouse_event.h"
#include "events/stop_event.h"
#include "utils/auto_release.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/formatter.h"

namespace
{

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB{};
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB{};

auto g_event_queue = std::queue<ufps::Event>{};

auto APIENTRY opengl_debug_callback(
    ::GLenum source,
    ::GLenum type,
    ::GLuint id,
    ::GLenum severity,
    ::GLsizei,
    const ::GLchar *message,
    const void *) -> void
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        throw ufps::Exception("{} {} {} {} {}", source, type, id, severity, message);
    }
}

auto CALLBACK window_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (Msg)
    {
        case WM_CLOSE: g_event_queue.emplace(ufps::StopEvent{}); break;
        case WM_KEYUP:
        {
            g_event_queue.emplace(ufps::KeyEvent{static_cast<ufps::Key>(wParam), ufps::KeyState::UP});
            break;
        }
        case WM_KEYDOWN:
        {
            g_event_queue.emplace(ufps::KeyEvent{static_cast<ufps::Key>(wParam), ufps::KeyState::DOWN});
            break;
        }
        case WM_INPUT:
        {
            auto raw = ::RAWINPUT{};
            auto dwSize = ::UINT{sizeof(::RAWINPUT)};
            ufps::ensure(
                ::GetRawInputData(
                    reinterpret_cast<::HRAWINPUT>(lParam), RID_INPUT, &raw, &dwSize, sizeof(::RAWINPUTHEADER)) !=
                    static_cast<::UINT>(-1),
                "failed to get raw input");

            if (raw.header.dwType == RIM_TYPEMOUSE)
            {
                const auto x = raw.data.mouse.lLastX;
                const auto y = raw.data.mouse.lLastY;

                g_event_queue.emplace(ufps::MouseEvent{static_cast<float>(x), static_cast<float>(y)});
            }

            break;
        }
        case WM_LBUTTONUP:
        {
            g_event_queue.emplace(
                ufps::MouseButtonEvent{
                    static_cast<float>(GET_X_LPARAM(lParam)),
                    static_cast<float>(GET_Y_LPARAM(lParam)),
                    ufps::MouseButtonState::UP});
            break;
        }
        case WM_LBUTTONDOWN:
        {
            g_event_queue.emplace(
                ufps::MouseButtonEvent{
                    static_cast<float>(GET_X_LPARAM(lParam)),
                    static_cast<float>(GET_Y_LPARAM(lParam)),
                    ufps::MouseButtonState::DOWN});
            break;
        }
    };

    return ::DefWindowProc(hWnd, Msg, wParam, lParam);
}

template <class T>
auto resolve_gl_function(T &function, const std::string &name) -> void
{
    const auto address = ::wglGetProcAddress(name.c_str());
    ufps::ensure(address != nullptr, "could not resolve {}", name);

    std::memcpy(std::addressof(function), &address, sizeof(T));
}

auto resolve_wgl_functions(::HINSTANCE instance) -> void
{
    auto wc = ::WNDCLASSA{};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = ::DefWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = "dummy window";

    const auto clazz = ::RegisterClassA(&wc);
    ufps::ensure(clazz != 0, "could not register dummy window");

    auto dummy_window = ufps::AutoRelease<::HWND>{
        ::CreateWindowExA(
            0,
            wc.lpszClassName,
            wc.lpszClassName,
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            wc.hInstance,
            0),
        ::DestroyWindow};

    ufps::ensure(dummy_window, "could not create dummy window");

    auto dc =
        ufps::AutoRelease<HDC>{::GetDC(dummy_window), [&dummy_window](auto dc) { ::ReleaseDC(dummy_window, dc); }};
    ufps::ensure(dc, "could not get dummy dc");

    auto pfd = ::PIXELFORMATDESCRIPTOR{};
    pfd.nSize = sizeof(::PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    auto pixel_format = ::ChoosePixelFormat(dc, &pfd);
    ufps::ensure(pixel_format != 0, "failed to choose pixel format");

    ufps::ensure(::SetPixelFormat(dc, pixel_format, &pfd) == TRUE, "failed to set pixel format");

    const auto context = ufps::AutoRelease<HGLRC>{::wglCreateContext(dc), ::wglDeleteContext};
    ufps::ensure(context, "failed to create wgl context");

    ufps::ensure(::wglMakeCurrent(dc, context) == TRUE, "could not make current context");

    resolve_gl_function(wglCreateContextAttribsARB, "wglCreateContextAttribsARB");
    resolve_gl_function(wglChoosePixelFormatARB, "wglChoosePixelFormatARB");

    ufps::ensure(::wglMakeCurrent(dc, 0) == TRUE, "could not unbind context");
}

auto init_opengl(HDC dc) -> void
{
    int pixel_format_attribs[]{
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_ACCELERATION_ARB,
        WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        32,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,
        WGL_SAMPLE_BUFFERS_ARB,
        GL_FALSE,
        WGL_SAMPLES_ARB,
        0,
        0};

    auto pixel_format = 0;
    auto num_formats = UINT{};

    ::wglChoosePixelFormatARB(dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    ufps::ensure(num_formats != 0u, "failed to choose a pixel format");

    auto pfd = ::PIXELFORMATDESCRIPTOR{};
    ufps::ensure(::DescribePixelFormat(dc, pixel_format, sizeof(pfd), &pfd) != 0, "failed to describe pixel format");
    ufps::ensure(::SetPixelFormat(dc, pixel_format, &pfd) == TRUE, "failed to set pixel format");

    int gl_attribs[]{
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        4,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        6,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    auto context = ::wglCreateContextAttribsARB(dc, 0, gl_attribs);
    ufps::ensure(context != nullptr, "failed to create wgl context");

    ufps::ensure(::wglMakeCurrent(dc, context) == TRUE, "failed to make current context");
}

auto resolve_global_gl_functions() -> void
{
#define RESOLVE(TYPE, NAME) resolve_gl_function(NAME, #NAME);

    FOR_OPENGL_FUNCTIONS(RESOLVE)
}

auto setup_debug() -> void
{
    ::glEnable(GL_DEBUG_OUTPUT);
    ::glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    ::glDebugMessageCallback(opengl_debug_callback, nullptr);
}

auto get_monitor_info(::HWND window) -> ::MONITORINFO
{
    auto mi = ::MONITORINFO{};
    mi.cbSize = sizeof(::MONITORINFO);

    ufps::ensure(
        ::GetMonitorInfo(::MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &mi) != 0,
        "could not get monitor info: {}",
        ::GetLastError());

    return mi;
}
}

namespace ufps
{

Window::Window(
    WindowMode mode,
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t x,
    std::uint32_t y,
    bool mouse_locked)
    : window_{}
    , dc_{}
    , wc_{}
    , width_{width}
    , height_{height}
    , mode_{mode}
    , mouse_locked_{mouse_locked}
{
    wc_ = ::WNDCLASS{
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = window_proc,
        .cbClsExtra = {},
        .cbWndExtra = {},
        .hInstance = ::GetModuleHandleA(nullptr),
        .hIcon = {},
        .hCursor{},
        .hbrBackground = {},
        .lpszMenuName = {},
        .lpszClassName = "window class"};

    ensure(::RegisterClassA(&wc_) != 0, "failed to register class");

    auto rect = ::RECT{.left = {}, .top = {}, .right = static_cast<int>(width), .bottom = static_cast<int>(height)};

    ensure(::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false) != 0, "failed to resize window");

    window_ = {
        ::CreateWindowExA(
            0,
            wc_.lpszClassName,
            "ufps window",
            WS_OVERLAPPEDWINDOW,
            x,
            y,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            wc_.hInstance,
            nullptr),
        ::DestroyWindow};

    dc_ = ufps::AutoRelease<HDC>{::GetDC(window_), [this](auto dc) { ::ReleaseDC(window_, dc); }};

    ::ShowWindow(window_, SW_SHOW);
    ::UpdateWindow(window_);

    ::GetWindowRect(window_, &rect);
    ::ClipCursor(&rect);
    ::ShowCursor(FALSE);

    const auto rid = ::RAWINPUTDEVICE{
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage = HID_USAGE_GENERIC_MOUSE,
        .dwFlags = RIDEV_INPUTSINK,
        .hwndTarget = window_};

    ensure(::RegisterRawInputDevices(&rid, 1, sizeof(rid)) == TRUE, "failed to register input device");

    resolve_wgl_functions(wc_.hInstance);
    init_opengl(dc_);
    resolve_global_gl_functions();

    if (config::opengl_debug_enabled)
    {
        setup_debug();
    }

    ::glEnable(GL_DEPTH_TEST);
    ::glEnable(GL_BLEND);
    ::glEnable(GL_MULTISAMPLE);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    set_mode(mode);

    const auto *vendor = ::glGetString(GL_VENDOR);
    const auto *renderer = ::glGetString(GL_RENDERER);
    const auto *version = ::glGetString(GL_VERSION);

    log::info(
        "created new window (dpi: {}) {} {} {} {} {} {}",
        ::GetDpiForWindow(window_),
        width_,
        height_,
        mode_,
        reinterpret_cast<const char *>(vendor),
        reinterpret_cast<const char *>(renderer),
        reinterpret_cast<const char *>(version));
}

auto Window::pump_event() const -> std::optional<Event>
{
    auto message = ::MSG{};
    while (::PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE) != 0)
    {
        ::TranslateMessage(&message);
        ::DispatchMessageA(&message);
    }

    if (!std::ranges::empty(g_event_queue))
    {
        const auto event = g_event_queue.front();
        g_event_queue.pop();
        return event;
    }

    return {};
}

auto Window::swap() const -> void
{
    ::SwapBuffers(dc_);

    if (mouse_locked_)
    {
        auto rect = ::RECT{};
        ::GetWindowRect(window_, &rect);
        ::SetCursorPos(rect.left + width_ / 2, rect.top + height_ / 2);
    }
}

auto Window::native_handle() const -> HWND
{
    return window_;
}

auto Window::render_width() const -> std::uint32_t
{
    return width_;
}

auto Window::render_height() const -> std::uint32_t
{
    return height_;
}

auto Window::window_width() const -> std::uint32_t
{
    if (mode_ == WindowMode::WINDOWED)
    {
        return render_width();
    }

    const auto mi = get_monitor_info(window_);
    return mi.rcMonitor.right - mi.rcMonitor.left;
}

auto Window::window_height() const -> std::uint32_t
{
    if (mode_ == WindowMode::WINDOWED)
    {
        return render_height();
    }

    const auto mi = get_monitor_info(window_);
    return mi.rcMonitor.bottom - mi.rcMonitor.top;
}

auto Window::set_title(const std::string &title) const -> void
{
    ::SetWindowTextA(window_, title.c_str());
}

auto Window::mode() const -> WindowMode
{
    return mode_;
}

auto Window::set_mode(WindowMode mode) -> void
{
    static auto wp_prev = ::WINDOWPLACEMENT{};
    wp_prev.length = sizeof(::WINDOWPLACEMENT);

    mode_ = mode;

    const auto current_style = ::GetWindowLong(window_, GWL_STYLE);
    ensure(current_style != 0, "could not get window style");

    if (current_style & WS_OVERLAPPEDWINDOW)
    {
        if (mode == WindowMode::WINDOWED)
        {
            return;
        }

        const auto mi = get_monitor_info(window_);
        ensure(::GetWindowPlacement(window_, &wp_prev) != 0, "could not get window placement");
        ensure(
            ::SetWindowLong(window_, GWL_STYLE, current_style & ~WS_OVERLAPPEDWINDOW) != 0,
            "could not set window style");
        ensure(
            ::SetWindowPos(
                window_,
                HWND_TOP,
                mi.rcMonitor.left,
                mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED) != 0,
            "failed to set window pos");
    }
    else
    {
        if (mode == WindowMode::FULLSCREEN)
        {
            return;
        }

        ensure(
            ::SetWindowLong(window_, GWL_STYLE, current_style | WS_OVERLAPPEDWINDOW) != 0,
            "failed to set window style");
        ensure(::SetWindowPlacement(window_, &wp_prev) != 0, "failed to set window placement");
        ensure(
            ::SetWindowPos(
                window_,
                nullptr,
                0,
                0,
                0,
                0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED) != 0,
            "failed to set window pos");
    }
}

}
