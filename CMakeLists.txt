cmake_minimum_required(VERSION 3.22)

project(
  ufps
  VERSION "2026.11.7.0"
  DESCRIPTION "a sweet fps game"
  LANGUAGES C CXX
)

set(CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)

option(UFPS_USE_EMBEDDED_RESOURCE_LOADER "Use EmbeddedResourceLoader" OFF)
option(UFPS_RESOURCE_PACKER_ONLY "Build only the native resource packer" OFF)

add_subdirectory(third_party)

if(NOT UFPS_RESOURCE_PACKER_ONLY)
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
else()
  add_subdirectory(tools)
endif()

