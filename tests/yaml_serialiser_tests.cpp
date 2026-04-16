#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "serialisation/yaml_serialiser.h"

struct Simple
{
    int a;

    auto operator==(const Simple &) const -> bool = default;
};

struct MultiMember
{
    int a;
    float b;
    std::string c;
    bool d;

    auto operator==(const MultiMember &) const -> bool = default;
};

struct Array
{
    std::vector<int> a;

    auto operator==(const Array &) const -> bool = default;
};

struct ArrayOfStruct
{
    std::vector<Simple> v;

    auto operator==(const ArrayOfStruct &) const -> bool = default;
};

struct Map
{
    std::unordered_map<std::string, int> a;

    auto operator==(const Map &) const -> bool = default;
};

struct NestedStruct
{
    MultiMember m;

    auto operator==(const NestedStruct &) const -> bool = default;
};

enum class Fruit
{
    APPLE
};

struct FruitStruct
{
    Fruit f;

    auto operator==(const FruitStruct &) const -> bool = default;
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

TEST(yaml_deserialisation, simple_struct)
{
    const auto yaml =
        R"(Simple:
  a: 12)";

    const auto result = ufps::yaml::deserialise<Simple>(yaml);
    const auto expected = Simple{.a = 12};

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, multi_member_struct)
{
    const auto yaml =
        R"(MultiMember:
  a: 12
  b: 3.1
  c: hello world
  d: true)";
    const auto result = ufps::yaml::deserialise<MultiMember>(yaml);
    const auto expected = MultiMember{
        .a = 12,
        .b = 3.1,
        .c = "hello world",
        .d = true,
    };

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, array_struct)
{
    const auto yaml =
        R"(Array:
   a:
     - 1
     - 2
     - 3
     - 4
     - 5
     - 7)";
    const auto result = ufps::yaml::deserialise<Array>(yaml);
    const auto expected = Array{.a = {1, 2, 3, 4, 5, 7}};

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, array_of_member_struct)
{
    const auto yaml =
        R"(ArrayOfStruct:
   v:
     - Simple:
         a: 10
     - Simple:
         a: 20
     - Simple:
         a: 30)";
    const auto result = ufps::yaml::deserialise<ArrayOfStruct>(yaml);
    const auto expected = ArrayOfStruct{
        .v =
            {
                {.a = 10},
                {.a = 20},
                {.a = 30},
            },
    };

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, map_struct)
{
    const auto yaml =
        R"(Map:
   a:
     2: 2
     3: 3
     1: 1)";
    const auto result = ufps::yaml::deserialise<Map>(yaml);
    const auto expected = Map{
        .a =
            {
                {"1", 1},
                {"2", 2},
                {"3", 3},
            },
    };

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, nested_struct)
{
    const auto yaml =
        R"(NestedStruct:
   m:
     MultiMember:
       a: 12
       b: 3.1
       c: hello world
       d: true)";
    const auto result = ufps::yaml::deserialise<NestedStruct>(yaml);
    const auto expected = NestedStruct{
        .m = {
            .a = 12,
            .b = 3.1,
            .c = "hello world",
            .d = true,
        }};

    ASSERT_EQ(result, expected);
}

TEST(yaml_deserialisation, enum_struct)
{
    const auto yaml =
        R"(FruitStruct:
   f: APPLE)";
    const auto result = ufps::yaml::deserialise<FruitStruct>(yaml);
    const auto expected = FruitStruct{.f = Fruit::APPLE};

    ASSERT_EQ(result, expected);
}
