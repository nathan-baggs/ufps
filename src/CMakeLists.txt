add_library(imguilib STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp

  ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)
target_include_directories(imguilib PUBLIC ${imgui_SOURCE_DIR} ${imguizmo_SOURCE_DIR})

add_library(ufpslib STATIC
)

add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(graphics)
add_subdirectory(resources)
add_subdirectory(utils)

add_executable(ufps
  main.cpp
)

install(TARGETS ufps DESTINATION bin)

configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)

target_compile_options(ufpslib PRIVATE
  -Wall
  -Wextra
  -pedantic
  -Werror
  -Wconversion-null
  -Wmissing-declarations
  -Woverlength-strings
  -Wpointer-arith
  -Wunused-local-typedefs
  -Wunused-result
  -Wvarargs
  -Wvla
  -Wwrite-strings
)
target_include_directories(ufpslib PUBLIC ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/src ${stb_lib_SOURCE_DIR})
target_compile_features(ufpslib PUBLIC cxx_std_23)
target_compile_definitions(ufpslib PUBLIC -DNOMINMAX)
target_link_libraries(ufpslib PUBLIC stdc++exp wbemuuid opengl32 imguilib dwmapi)

target_link_libraries(ufps PUBLIC ufpslib)

