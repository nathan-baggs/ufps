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

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
add_subdirectory(assimp EXCLUDE_FROM_ALL)
target_compile_options(assimp PRIVATE -Wno-error=maybe-uninitialized)
