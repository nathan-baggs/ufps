#include <array>
#include <cstring>
#include <numbers>

#include <gtest/gtest.h>

#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "maths/vector4.h"
#include "utils/exception.h"

TEST(matrix4, identity_ctor)
{
    const auto m = ufps::Matrix4{};

    const auto expected = std::array<float, 16u>{
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

    const auto result = m.data();
    ASSERT_EQ(result.size(), expected.size());
    ASSERT_EQ(std::memcmp(result.data(), expected.data(), result.size_bytes()), 0);
}

TEST(matrix4, element_ctor)
{
    const auto expected = std::array<float, 16u>{
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};

    const auto m = ufps::Matrix4{expected};

    const auto result = m.data();
    ASSERT_EQ(result.size(), expected.size());
    ASSERT_EQ(std::memcmp(result.data(), expected.data(), result.size_bytes()), 0);
}

TEST(matrix4, incorrect_element_size_ctor)
{
    const auto expected = std::array<float, 1u>{{1.0f}};

    EXPECT_THROW(ufps::Matrix4{expected}, ufps::Exception);
}

TEST(matrix4, multiply)
{
    const auto m1 = ufps::Matrix4{
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};
    const auto m2 = ufps::Matrix4{
        {1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 15.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f, 16.5f}};
    const auto expected = ufps::Matrix4{
        {104.0f,
         116.0f,
         128.0f,
         140.0f,
         216.0f,
         244.0f,
         272.0f,
         300.0f,
         353.0f,
         402.0f,
         451.0f,
         500.0f,
         440.0f,
         500.0f,
         560.0f,
         620.0f}};
    ASSERT_EQ(m1 * m2, expected);
}

TEST(matrix4, multiply_assign)
{
    auto m = ufps::Matrix4{
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};
    m *= ufps::Matrix4{
        {1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f, 15.5f, 11.5f, 12.5f, 13.5f, 14.5f, 15.5f, 16.5f}};
    const auto expected = ufps::Matrix4{
        {104.0f,
         116.0f,
         128.0f,
         140.0f,
         216.0f,
         244.0f,
         272.0f,
         300.0f,
         353.0f,
         402.0f,
         451.0f,
         500.0f,
         440.0f,
         500.0f,
         560.0f,
         620.0f}};
    ASSERT_EQ(m, expected);
}

TEST(matrix4, look_at)
{
    const auto view = ufps::Matrix4::look_at({1.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

    const auto expected = ufps::Matrix4{
        {0.9805f,
         0.0000f,
         0.1961f,
         0.0000f,
         0.0000f,
         1.000000f,
         -0.0000f,
         0.0000f,
         -0.1961f,
         0.0000f,
         0.9805f,
         0.0000f,
         -0.0000f,
         -0.0000f,
         -5.0990f,
         1.0000f}};

    const auto view_spn = view.data();
    const auto expected_spn = expected.data();

    for (auto i = 0u; i < 16u; ++i)
    {
        ASSERT_NEAR(view_spn[i], expected_spn[i], 0.001f);
    }
}

TEST(matrix4, perspective)
{
    const auto proj = ufps::Matrix4::perspective(std::numbers::pi_v<float> / 4.0f, 800.0f, 600.0f, 0.1f, 100.0f);

    const auto expected = ufps::Matrix4{
        {1.810660f,
         0.000000f,
         0.000000f,
         0.000000f,
         0.000000f,
         2.414213f,
         0.000000f,
         0.000000f,
         0.000000f,
         0.000000f,
         -1.002002f,
         -1.000000f,
         0.000000f,
         0.000000f,
         -0.200200f,
         0.000000f}};

    const auto proj_spn = proj.data();
    const auto expected_spn = expected.data();

    for (auto i = 0u; i < 16u; ++i)
    {
        ASSERT_NEAR(proj_spn[i], expected_spn[i], 0.001f);
    }
}

TEST(matrix4, invert)
{
    const auto m = ufps::Matrix4{
        {0.6f, 2.4f, 1.1f, 0.0f, 2.4f, 0.6f, -0.4f, 1.0f, 1.1f, -0.4f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

    const auto inv = ufps::Matrix4::invert(m);

    const auto expected = ufps::Matrix4{
        {-0.0323939,
         0.304503,
         0.262391,
         -0.304503,
         0.304503,
         0.137674,
         -0.466472,
         -0.137674,
         0.262391,
         -0.466472,
         0.874636,
         0.466472,
         0,
         0,
         0,
         1}};

    const auto inv_spn = inv.data();
    const auto expected_spn = expected.data();

    for (auto i = 0u; i < 16u; ++i)
    {
        ASSERT_NEAR(inv_spn[i], expected_spn[i], 0.001f);
    }
}

TEST(matrix4, multiply_vector4)
{
    const auto m = ufps::Matrix4{
        {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f}};
    const auto v = ufps::Vector4{1.0f, 2.0f, 3.0f, 4.0f};

    const auto result = m * v;

    const auto expected = ufps::Vector4{90.0f, 100.0f, 110.0f, 120.0f};

    ASSERT_EQ(result, expected);
}
