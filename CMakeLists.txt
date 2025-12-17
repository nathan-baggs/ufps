cmake_minimum_required(VERSION 3.22)

project(
  ufps
  VERSION "0.0.15"
  DESCRIPTION "a sweet fps game"
  LANGUAGES C CXX
)

include(FetchContent)

set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)
FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.92.5)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
	imguizmo
	GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
	GIT_TAG df1c301)
FetchContent_MakeAvailable(imguizmo)

add_subdirectory(src)

enable_testing()
include(CTest)
add_subdirectory(tests)

set(CPACK_PACKAGE_NAME "ufps")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_SOURCE_IGNORE_FILES "build")
include(CPack)

