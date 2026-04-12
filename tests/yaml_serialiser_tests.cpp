#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "serialisation/yaml_serialiser.h"

struct Simple
{
    int a;
};

struct MultiMember
{
    int a;
    float b;
    std::string c;
    bool d;
};

struct Array
{
    std::vector<int> a;
};

struct ArrayOfStruct
{
    std::vector<Simple> v;
};

struct Map
{
    std::unordered_map<std::string, int> a;
};

// arrays, arrays of structs, maps, maps of structs, nested structs, enums

TEST(yaml_serialisation, simple_struct)
{
    const auto result = ufps::yaml::serialise(Simple{.a = 12});
    const auto expected =
        R"(Simple:
  a: 12)";

    ASSERT_EQ(result, expected);
}

TEST(yaml_serialisation, multi_member_struct)
{
    const auto result = ufps::yaml::serialise(
        MultiMember{
            .a = 12,
            .b = 3.1,
            .c = "hello world",
            .d = true,
        });
    const auto expected =
        R"(MultiMember:
  a: 12
  b: 3.1
  c: hello world
  d: true)";

    ASSERT_EQ(result, expected);
}

TEST(yaml_serialisation, array_struct)
{
    const auto result = ufps::yaml::serialise(Array{.a = {1, 2, 3, 4, 5, 7}});
    const auto expected =
        R"(Array:
  a:
    - 1
    - 2
    - 3
    - 4
    - 5
    - 7)";

    ASSERT_EQ(result, expected);
}

TEST(yaml_serialisation, array_of_member_struct)
{
    const auto result = ufps::yaml::serialise(
        ArrayOfStruct{
            .v =
                {
                    {.a = 10},
                    {.a = 20},
                    {.a = 30},
                },
        });
    const auto expected =
        R"(ArrayOfStruct:
  v:
    - a: 10
    - a: 20
    - a: 30)";

    ASSERT_EQ(result, expected);
}

TEST(yaml_serialisation, map_struct)
{
    const auto result = ufps::yaml::serialise(
        Map{
            .a =
                {
                    {"1", 1},
                    {"2", 2},
                    {"3", 3},
                },
        });
    const auto expected =
        R"(Map:
  a:
    2: 2
    3: 3
    1: 1)";

    ASSERT_EQ(result, expected);
}
