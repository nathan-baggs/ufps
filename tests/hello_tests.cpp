#include <string_view>

#include <gtest/gtest.h>

#include "utils/hello.h"

using namespace std::literals;

TEST(hello_tests, simple)
{
    ASSERT_EQ(ufps::hello(), "hello, world!"sv);
}
