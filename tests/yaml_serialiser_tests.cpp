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

struct NestedStruct
{
    MultiMember m;
};

enum class Fruit
{
    APPLE
};

struct FruitStruct
{
    Fruit f;
};

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
    - Simple:
        a: 10
    - Simple:
        a: 20
    - Simple:
        a: 30)";

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

TEST(yaml_serialisation, nested_struct)
{
    const auto result = ufps::yaml::serialise(
        NestedStruct{
            .m = {
                .a = 12,
                .b = 3.1,
                .c = "hello world",
                .d = true,
            }});
    const auto expected =
        R"(NestedStruct:
  m:
    MultiMember:
      a: 12
      b: 3.1
      c: hello world
      d: true)";

    ASSERT_EQ(result, expected);
}

TEST(yaml_serialisation, enum_struct)
{
    const auto result = ufps::yaml::serialise(FruitStruct{.f = Fruit::APPLE});
    const auto expected =
        R"(FruitStruct:
  f: APPLE)";

    ASSERT_EQ(result, expected);
}
