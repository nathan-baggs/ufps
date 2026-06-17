add_library(ufpslib STATIC
)

add_subdirectory(core)
add_subdirectory(concurrency)
add_subdirectory(graphics)
add_subdirectory(memory)
add_subdirectory(physics)
add_subdirectory(resources)
add_subdirectory(serialisation)
add_subdirectory(utils)

add_executable(ufps
  main.cpp
)
target_compile_options(ufps PRIVATE
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

install(TARGETS ufps DESTINATION bin)

configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)

target_compile_options(ufpslib PUBLIC
  -freflection
)

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
  -Winvalid-pch
  -fcontract-evaluation-semantic=enforce
)

target_include_directories(ufpslib PUBLIC ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/src ${zstd_SOURCE_DIR}/lib)
target_compile_features(ufpslib PUBLIC cxx_std_26)
target_compile_definitions(ufpslib PUBLIC -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
target_link_libraries(ufpslib PUBLIC stdc++exp wbemuuid opengl32 imguilib stblib dwmapi assimp yaml-cpp libzstd_static dbghelp libbacktrace Jolt)
target_link_options(ufps PUBLIC "LINKER:--threads=8,--icf=safe")
target_link_libraries(ufps PUBLIC ufpslib)

