FetchContent_MakeAvailable(googletest)

include(GoogleTest)

mark_as_advanced(BUILD_GMOCK BUILD_GTEST gtest_hide_internal_symbols)

add_executable(unit_tests
  auto_release_tests.cpp
  awaitable_manager_tests.cpp
  concurrent_queue_tests.cpp
  error_tests.cpp
  formatter_tests.cpp
  matrix3_tests.cpp
  matrix4_tests.cpp
  multi_buffer_tests.cpp
  sparse_set_tests.cpp
  task_tests.cpp
  thread_pool_tests.cpp
  thread_tests.cpp
  vector3_tests.cpp
  yaml_serialiser_tests.cpp
)

target_link_libraries(unit_tests gmock_main ufpslib)
gtest_discover_tests(unit_tests DISCOVERY_MODE PRE_TEST)

