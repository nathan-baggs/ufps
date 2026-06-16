cmake_minimum_required(VERSION 3.22)

project(
  ufps
  VERSION "2026.6.5.0"
  DESCRIPTION "a sweet fps game"
  LANGUAGES C CXX
)

include(FetchContent)
include(ExternalProject)

set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)

option(UFPS_USE_EMBEDDED_RESOURCE_LOADER "Use EmbeddedResourceLoader" OFF)

set(BACKTRACE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/_deps/libbacktrace")
set(BACKTRACE_INSTALL_DIR "${BACKTRACE_PREFIX}/install")
set(BACKTRACE_LIB_PATH "${BACKTRACE_INSTALL_DIR}/lib/libbacktrace.a")

ExternalProject_Add(
    libbacktrace_build
    GIT_REPOSITORY    https://github.com/ianlancetaylor/libbacktrace.git
    GIT_TAG           master
    PREFIX            "${BACKTRACE_PREFIX}"
    UPDATE_COMMAND    ""
    BUILD_BYPRODUCTS  "${BACKTRACE_LIB_PATH}"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure 
                      --prefix=${BACKTRACE_INSTALL_DIR} 
                      --with-pic
                      --host=x86_64-w64-mingw32
                      CC=${CMAKE_C_COMPILER}
                      CXX=${CMAKE_CXX_COMPILER}
    BUILD_COMMAND     make
    INSTALL_COMMAND   make install
)
add_library(libbacktrace STATIC IMPORTED)
add_dependencies(libbacktrace libbacktrace_build)
file(MAKE_DIRECTORY "${BACKTRACE_INSTALL_DIR}/include")
set_target_properties(libbacktrace PROPERTIES
    IMPORTED_LOCATION "${BACKTRACE_INSTALL_DIR}/lib/libbacktrace.a"
    INTERFACE_INCLUDE_DIRECTORIES "${BACKTRACE_INSTALL_DIR}/include"
)

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

