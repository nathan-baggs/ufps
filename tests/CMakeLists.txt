FetchContent_MakeAvailable(googletest)

include(GoogleTest)

mark_as_advanced(BUILD_GMOCK BUILD_GTEST gtest_hide_internal_symbols)

add_executable(unit_tests
  auto_release_tests.cpp
  error_tests.cpp
  formatter_tests.cpp
  multi_buffer_tests.cpp
  matrix4_tests.cpp
  vector3_tests.cpp
)

target_link_libraries(unit_tests gmock_main ufpslib)
gtest_discover_tests(unit_tests DISCOVERY_MODE PRE_TEST)

