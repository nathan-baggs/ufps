cmake_minimum_required(VERSION 3.22)

project(
  ufps
  VERSION "2026.6.5.0"
  DESCRIPTION "a sweet fps game"
  LANGUAGES C CXX
)

include(FetchContent)

set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)

option(UFPS_USE_EMBEDDED_RESOURCE_LOADER "Use EmbeddedResourceLoader" OFF)

FetchContent_Declare(
	jolt
	GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
	GIT_TAG v5.5.0
	SOURCE_SUBDIR Build)
FetchContent_GetProperties(jolt)
if (NOT jolt_POPULATED)
	FetchContent_Populate(jolt)
	add_subdirectory(${jolt_SOURCE_DIR}/Build ${jolt_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_compile_options(Jolt PRIVATE -Wno-error=deprecated-declarations)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(tools)

enable_testing()
include(CTest)
add_subdirectory(tests)

set(CPACK_PACKAGE_NAME "ufps")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_SOURCE_IGNORE_FILES "build")
include(CPack)

