add_subdirectory(googletest EXCLUDE_FROM_ALL)

add_library(imguilib STATIC
  imgui/imgui.cpp
  imgui/backends/imgui_impl_opengl3.cpp
  imgui/backends/imgui_impl_win32.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp

  ImGuizmo/ImGuizmo.cpp
)
target_include_directories(imguilib PUBLIC imgui ImGuizmo)

add_library(stblib INTERFACE)
target_include_directories(stblib INTERFACE stb)

