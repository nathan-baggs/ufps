add_executable(resource_packer
  assimp_loader.cpp
  resource_packer.cpp
)

target_compile_options(resource_packer PRIVATE
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
  -Wno-missing-declarations
)

target_link_libraries(resource_packer PRIVATE assimp)

if(UFPS_RESOURCE_PACKER_ONLY)
  target_sources(resource_packer PRIVATE
    file_resource_loader.cpp
    "${PROJECT_SOURCE_DIR}/src/utils/compress.cpp"
    "${PROJECT_SOURCE_DIR}/src/animation/skeleton.cpp"
  )

  configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  )
  target_include_directories(resource_packer PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
  )
  target_compile_features(resource_packer PRIVATE cxx_std_26)
  target_compile_options(resource_packer PRIVATE
    -freflection
    -fcontract-evaluation-semantic=enforce
  )
  find_package(Threads REQUIRED)
  target_link_libraries(resource_packer PRIVATE
    yaml-cpp
    libzstd_static
    stdc++exp
    Threads::Threads
  )
else()
  target_link_libraries(resource_packer PRIVATE ufpslib)
endif()
