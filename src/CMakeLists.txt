add_library(ufpslib STATIC
)

add_subdirectory(events)
add_subdirectory(graphics)
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
  -Wcast-qual
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
target_include_directories(ufpslib PUBLIC ${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/src)
target_compile_features(ufpslib PUBLIC cxx_std_23)
target_compile_definitions(ufpslib PUBLIC -DNOMINMAX)
target_link_libraries(ufpslib PUBLIC stdc++exp wbemuuid opengl32)

target_link_libraries(ufps PUBLIC ufpslib)

