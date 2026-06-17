set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)

include(ExternalProject)

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
set(ASSIMP_IGNORE_GIT_HASH ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp EXCLUDE_FROM_ALL)
target_compile_options(assimp PRIVATE -Wno-error=maybe-uninitialized)

set(YAML_CPP_BUILD_CONTRIB OFF)
set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_INSTALL OFF)
set(YAML_CPP_FORMAT_SOURCE OFF)
add_subdirectory(yaml-cpp EXCLUDE_FROM_ALL)

set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
add_subdirectory(zstd/build/cmake EXCLUDE_FROM_ALL)

set(BACKTRACE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace")
set(BACKTRACE_INSTALL_DIR "${BACKTRACE_PREFIX}/install")
file(TO_CMAKE_PATH "${BACKTRACE_INSTALL_DIR}" BACKTRACE_INSTALL_DIR)
set(BACKTRACE_LIB_PATH "${BACKTRACE_INSTALL_DIR}/lib/libbacktrace.a")
file(MAKE_DIRECTORY "${BACKTRACE_INSTALL_DIR}/include")
ExternalProject_Add(
    libbacktrace_build
    SOURCE_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace"
    PREFIX            "${BACKTRACE_PREFIX}"
    UPDATE_COMMAND    ""
    BUILD_BYPRODUCTS  "${BACKTRACE_LIB_PATH}"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      "--prefix=${BACKTRACE_INSTALL_DIR}" 
                      --with-pic
                      --host=x86_64-w64-mingw32
                      "CC=${CMAKE_C_COMPILER}"
                      "CXX=${CMAKE_CXX_COMPILER}"
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
)
add_library(libbacktrace INTERFACE)
add_dependencies(libbacktrace libbacktrace_build)
target_include_directories(libbacktrace INTERFACE "${BACKTRACE_INSTALL_DIR}/include")
target_link_libraries(libbacktrace INTERFACE "${BACKTRACE_LIB_PATH}")

# Disable Jolt features that bloat compilation or binary size
set(PROFILER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF CACHE BOOL "" FORCE)
set(ENABLE_OBJECT_STREAM OFF CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(GENERATE_DEBUG_SYMBOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(JoltPhysics/Build)

set(CMAKE_DISABLE_PRECOMPILE_HEADERS OFF)
