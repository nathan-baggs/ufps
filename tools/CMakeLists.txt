add_executable(resource_packer
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
)

target_link_libraries(resource_packer
   ufpslib
)
