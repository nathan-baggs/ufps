#include <cmath>
#include <format>

#include <gtest/gtest.h>

#include "maths/vector3.h"
#include "utils/formatter.h"

TEST(vector3, empty_ctor)
{
    const auto v = ufps::Vector3{};

    ASSERT_EQ(v.x, 0.0f);
    ASSERT_EQ(v.y, 0.0f);
    ASSERT_EQ(v.z, 0.0f);
}

TEST(vector3, single_value_ctor)
{
    const auto v = ufps::Vector3{1.1f};

    ASSERT_EQ(v.x, 1.1f);
    ASSERT_EQ(v.y, 1.1f);
    ASSERT_EQ(v.z, 1.1f);
}

TEST(vector3, all_components_ctor)
{
    const auto v = ufps::Vector3{1.1f, 2.2f, 3.3f};

    ASSERT_EQ(v.x, 1.1f);
    ASSERT_EQ(v.y, 2.2f);
    ASSERT_EQ(v.z, 3.3f);
}

TEST(vector3, normalise)
{
    const auto length = std::sqrt(50.0f);
    const auto v = ufps::Vector3{3.0f, 4.0f, 5.0f};

    const auto n = ufps::Vector3::normalise(v);

    EXPECT_NEAR(n.x, v.x / length, 0.0001f);
    EXPECT_NEAR(n.y, v.y / length, 0.0001f);
    EXPECT_NEAR(n.z, v.z / length, 0.0001f);
}

TEST(vector3, cross)
{
    const auto v1 = ufps::Vector3{1.0f, 2.0f, 3.0f};
    const auto v2 = ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(ufps::Vector3::cross(v1, v2), ufps::Vector3(-3.0f, 6.0f, -3.0f));
}

TEST(vector3, addition)
{
    const auto v1 = ufps::Vector3{1.0f, 2.0f, 3.0f};
    const auto v2 = ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v1 + v2, ufps::Vector3(5.0f, 7.0f, 9.0f));
}

TEST(vector3, addition_assignment)
{
    auto v = ufps::Vector3{1.0f, 2.0f, 3.0f};
    v += ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v, ufps::Vector3(5.0f, 7.0f, 9.0f));
}

TEST(vector3, subtraction)
{
    const auto v1 = ufps::Vector3{2.0f, 10.0f, 12.0f};
    const auto v2 = ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v1 - v2, ufps::Vector3(-2.0f, 5.0f, 6.0f));
}

TEST(vector3, subtraction_assignment)
{
    auto v = ufps::Vector3{2.0f, 10.0f, 12.0f};
    v -= ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v, ufps::Vector3(-2.0f, 5.0f, 6.0f));
}

TEST(vector3, multiplication)
{
    const auto v1 = ufps::Vector3{2.0f, 10.0f, 12.0f};
    const auto v2 = ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v1 * v2, ufps::Vector3(8.0f, 50.0f, 72.0f));
}

TEST(vector3, multiplication_assignment)
{
    auto v = ufps::Vector3{2.0f, 10.0f, 12.0f};
    v *= ufps::Vector3{4.0f, 5.0f, 6.0f};

    ASSERT_EQ(v, ufps::Vector3(8.0f, 50.0f, 72.0f));
}

TEST(vector3, negate)
{
    const auto v1 = ufps::Vector3{1.0f, 2.0f, 3.0f};
    const auto expected = ufps::Vector3{-1.0f, -2.0f, -3.0f};
    ASSERT_EQ(-v1, expected);
}

TEST(vector3, to_string)
{
    const auto v = ufps::Vector3{1.0f, 2.0f, 3.0f};
    const auto str = v.to_string();
    ASSERT_EQ(str, "x=1 y=2 z=3");
}

TEST(vector3, format)
{
    const auto v = ufps::Vector3{1.0f, 2.0f, 3.0f};
    const auto str = std::format("{}", v);
    ASSERT_EQ(str, "x=1 y=2 z=3");
}
