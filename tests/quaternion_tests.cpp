#include <cmath>
#include <contracts>
#include <numbers>

#include <gtest/gtest.h>

#include "maths/matrix4.h"
#include "maths/quaternion.h"
#include "maths/vector4.h"

namespace
{

constexpr auto pi = std::numbers::pi_v<float>;
constexpr auto half_sqrt_two = 0.70710678118654752440f;

void expect_quaternion_near(const ufps::Quaternion &actual, const ufps::Quaternion &expected, float tolerance = 1.0e-5f)
{
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
    EXPECT_NEAR(actual.z, expected.z, tolerance);
    EXPECT_NEAR(actual.w, expected.w, tolerance);
}

void expect_vector_near(const ufps::Vector3 &actual, const ufps::Vector3 &expected, float tolerance = 1.0e-5f)
{
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
    EXPECT_NEAR(actual.z, expected.z, tolerance);
}

auto rotate(const ufps::Quaternion &rotation, const ufps::Vector3 &vector) -> ufps::Vector3
{
    const auto rotated = ufps::Matrix4{rotation} * ufps::Vector4{vector, 0.0f};
    return {rotated.x, rotated.y, rotated.z};
}

}

TEST(quaternion, default_ctor)
{
    const auto q = ufps::Quaternion{};
    ASSERT_FLOAT_EQ(q.x, 0.0f);
    ASSERT_FLOAT_EQ(q.y, 0.0f);
    ASSERT_FLOAT_EQ(q.z, 0.0f);
    ASSERT_FLOAT_EQ(q.w, 1.0f);
}

TEST(quaternion, value_ctor)
{
    const auto q = ufps::Quaternion{1.0f, 2.0f, 3.0f, 4.0f};
    ASSERT_FLOAT_EQ(q.x, 1.0f);
    ASSERT_FLOAT_EQ(q.y, 2.0f);
    ASSERT_FLOAT_EQ(q.z, 3.0f);
    ASSERT_FLOAT_EQ(q.w, 4.0f);
}

TEST(quaternion, axis_angle_ctor)
{
    const auto q = ufps::Quaternion{{0.0f, 0.0f, 1.0f}, pi / 2.0f};

    expect_quaternion_near(q, {0.0f, 0.0f, half_sqrt_two, half_sqrt_two});
    expect_vector_near(rotate(q, {1.0f, 0.0f, 0.0f}), {0.0f, 1.0f, 0.0f});
}

TEST(quaternion, vector_to_vector_ctor)
{
    const auto from = ufps::Vector3::normalise({1.0f, 2.0f, 3.0f});
    const auto to = ufps::Vector3::normalise({-3.0f, 1.0f, 2.0f});
    const auto q = ufps::Quaternion{from, to};

    EXPECT_NEAR(std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w), 1.0f, 1.0e-5f);
    expect_vector_near(rotate(q, from), to);
}

TEST(quaternion, vector_to_same_vector_ctor)
{
    const auto vector = ufps::Vector3::normalise({1.0f, 2.0f, 3.0f});
    const auto q = ufps::Quaternion{vector, vector};

    expect_quaternion_near(q, {});
    expect_vector_near(rotate(q, vector), vector);
}

TEST(quaternion, vector_to_opposite_vector_ctor)
{
    const auto from = ufps::Vector3{1.0f, 0.0f, 0.0f};
    const auto to = -from;
    const auto q = ufps::Quaternion{from, to};

    EXPECT_NEAR(std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w), 1.0f, 1.0e-5f);
    expect_vector_near(rotate(q, from), to);
}

TEST(quaternion, euler_ctor)
{
    expect_quaternion_near(ufps::Quaternion{pi / 2.0f, 0.0f, 0.0f}, {0.0f, half_sqrt_two, 0.0f, half_sqrt_two});
    expect_quaternion_near(ufps::Quaternion{0.0f, pi / 2.0f, 0.0f}, {half_sqrt_two, 0.0f, 0.0f, half_sqrt_two});
    expect_quaternion_near(ufps::Quaternion{0.0f, 0.0f, pi / 2.0f}, {0.0f, 0.0f, half_sqrt_two, half_sqrt_two});
}

TEST(quaternion, normalise)
{
    const auto q = ufps::Quaternion::normalise({1.0f, 2.0f, 3.0f, 4.0f});
    const auto length = std::sqrt(30.0f);

    expect_quaternion_near(q, {1.0f / length, 2.0f / length, 3.0f / length, 4.0f / length});
}

TEST(quaternion, multiply)
{
    const auto rotate_x = ufps::Quaternion{{1.0f, 0.0f, 0.0f}, pi / 2.0f};
    const auto rotate_y = ufps::Quaternion{{0.0f, 1.0f, 0.0f}, pi / 2.0f};
    const auto combined = rotate_y * rotate_x;

    expect_vector_near(rotate(combined, {0.0f, 0.0f, 1.0f}), {0.0f, -1.0f, 0.0f});
    expect_quaternion_near(ufps::Quaternion{} * rotate_x, rotate_x);
    expect_quaternion_near(rotate_y * ufps::Quaternion{}, rotate_y);
}

TEST(quaternion, multiply_assign)
{
    const auto rotate_x = ufps::Quaternion{{1.0f, 0.0f, 0.0f}, pi / 2.0f};
    const auto rotate_y = ufps::Quaternion{{0.0f, 1.0f, 0.0f}, pi / 2.0f};
    auto combined = rotate_y;

    auto &result = combined *= rotate_x;

    EXPECT_EQ(&result, &combined);
    expect_quaternion_near(combined, rotate_y * rotate_x);
}

TEST(quaternion, equality)
{
    const auto q = ufps::Quaternion{1.0f, 2.0f, 3.0f, 4.0f};
    const auto different = ufps::Quaternion{1.0f, 2.0f, 3.0f, 5.0f};

    EXPECT_EQ(q, q);
    EXPECT_NE(q, different);
}

TEST(quaternion, to_string)
{
    const auto q = ufps::Quaternion{1.0f, 2.0f, 3.0f, 4.0f};

    EXPECT_EQ(q.to_string(), "x=1 y=2 z=3 w=4");
}
