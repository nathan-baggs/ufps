#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

#include "maths/bounded_number.h"
#include "utils/exception.h"

namespace
{

template <auto Value, float Min, float Max>
concept IsValidBoundedNumber = requires {
    // check conversions are valid and shortcut the lambda if not
    ufps::BoundedFloat<Min, Max>{Value};

    typename std::bool_constant<[]
                                {
                                    ufps::BoundedFloat<Min, Max>{Value};
                                    return true;
                                }()>;
};

}

TEST(bounded_number, default_ctor)
{
    auto f = ufps::BoundedFloat<0.0f, 10.0f>{};
    ASSERT_FLOAT_EQ(f.value(), 0.0f);
    ASSERT_FLOAT_EQ(*f, 0.0f);

    static_assert(IsValidBoundedNumber<0.0f, 0.0f, 10.0f>);
    static_assert(*ufps::BoundedFloat<0.0f, 10.0f>{} == 0.0f);
    static_assert(ufps::BoundedFloat<0.0f, 10.0f>::min == 0.0f);
    static_assert(ufps::BoundedFloat<0.0f, 10.0f>::max == 10.0f);
    static_assert(std::same_as<decltype(f)::type, float>);
}

TEST(bounded_number, default_ctor_out_of_range)
{
    ASSERT_THROW((ufps::BoundedFloat<1.0f, 10.0f>{}), ufps::Exception);

    static_assert(!IsValidBoundedNumber<0.0f, 1.0f, 10.0f>);
}

TEST(bounded_number, value_ctor)
{
    auto f = ufps::BoundedFloat<0.0f, 10.0f>{2.2f};
    ASSERT_FLOAT_EQ(f.value(), 2.2f);
    ASSERT_FLOAT_EQ(*f, 2.2f);

    static_assert(IsValidBoundedNumber<2.2f, 0.0f, 10.0f>);
    static_assert(*ufps::BoundedFloat<0.0f, 10.0f>{2.2f} == 2.2f);
}

TEST(bounded_number, value_ctor_out_of_range)
{
    ASSERT_THROW((ufps::BoundedFloat<1.0f, 10.0f>{11.0f}), ufps::Exception);

    static_assert(!IsValidBoundedNumber<11.0f, 0.0f, 10.0f>);
}

TEST(bounded_number, equality)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{3};
    auto d = ufps::BoundedDouble<0.0, 10.0>{3};
    auto u = ufps::BoundedUint32<0u, 10u>{3};

    ASSERT_FLOAT_EQ(f, d);
    ASSERT_FLOAT_EQ(f, u);
    ASSERT_FLOAT_EQ(u, d);
}

TEST(bounded_number, inequality)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{3.3f};
    auto d = ufps::BoundedDouble<0.0, 10.0>{3.1};
    auto u = ufps::BoundedUint32<0u, 10u>{3};

    ASSERT_NE(f, d);
    ASSERT_NE(f, u);
    ASSERT_NE(u, d);
}

TEST(bounded_number, copy_ctor_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{3.0f};
    ufps::BoundedFloat<2.0f, 4.0f> f2{f1};

    ASSERT_FLOAT_EQ(*f2, 3.0f);
}

TEST(bounded_number, copy_ctor_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    ASSERT_THROW((ufps::BoundedFloat<2.0f, 4.0f>{f1}), ufps::Exception);
}

TEST(bounded_number, copy_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{3.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 4.0f>{2.0f};
    f2 = f1;

    ASSERT_FLOAT_EQ(*f2, 3.0f);
}

TEST(bounded_number, copy_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 4.0f>{2.0f};
    ASSERT_THROW((f2 = f1), ufps::Exception);
}

TEST(bounded_number, move_ctor_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{3.0f};
    ufps::BoundedFloat<2.0f, 4.0f> f2{std::move(f1)};

    ASSERT_FLOAT_EQ(*f2, 3.0f);
}

TEST(bounded_number, move_ctor_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    ASSERT_THROW((ufps::BoundedFloat<2.0f, 4.0f>{std::move(f1)}), ufps::Exception);
}

TEST(bounded_number, move_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{3.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 4.0f>{2.0f};
    f2 = std::move(f1);

    ASSERT_FLOAT_EQ(*f2, 3.0f);
}

TEST(bounded_number, move_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 4.0f>{2.0f};
    ASSERT_THROW((f2 = std::move(f1)), ufps::Exception);
}

TEST(bounded_number, addressof)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};

    float *p = &f;
    *p = 6.1f;

    ASSERT_FLOAT_EQ(*f, 6.1f);
}

TEST(bounded_number, cast)
{
    ASSERT_FLOAT_EQ(static_cast<float>(ufps::BoundedFloat<1.0f, 10.0f>{2.2f}), 2.2f);
}

TEST(bounded, unary_minus)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    auto f2 = -f1;

    ASSERT_FLOAT_EQ(*f2, -5.0f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == -10.0f);
    static_assert(decltype(f2)::max == -1.0f);
}

TEST(bounded, unary_plus)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    auto f2 = +f1;

    ASSERT_FLOAT_EQ(*f2, 5.0f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == 1.0f);
    static_assert(decltype(f2)::max == 10.0f);
}

TEST(bounded, unary_plus_integer_promotion)
{
    auto f1 = ufps::BoundedNumber<char, 1, 10>{5};
    auto f2 = +f1;

    ASSERT_FLOAT_EQ(f2, 5);

    static_assert(std::same_as<decltype(f2)::type, int>);
    static_assert(decltype(f2)::min == 1);
    static_assert(decltype(f2)::max == 10);
}

TEST(bounded_member, add_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.1f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{2.0f};
    auto f3 = f1 + f2;

    ASSERT_FLOAT_EQ(*f3, 4.1f);

    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == 3.0f);
    static_assert(decltype(f3)::max == 16.0f);
}

TEST(bounded_member, add_scalar)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.1f};
    auto f2 = 2.0f + f1;

    ASSERT_FLOAT_EQ(*f2, 4.1f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f2)::max == std::numeric_limits<float>::max());
}

TEST(bounded_number, add_smaller_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.2f};
    auto f2 = f1 + ufps::BoundedNumber<std::uint16_t, 2, 6>{3};

    ASSERT_FLOAT_EQ(*f2, 5.2f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == 3.0f);
    static_assert(decltype(f2)::max == 16.0f);
}

TEST(bounded_number, add_larger_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.5f};
    auto f2 = f1 + ufps::BoundedDouble<2.0, 6.0>{3.0};

    ASSERT_DOUBLE_EQ(*f2, 5.5);

    static_assert(std::same_as<decltype(f2)::type, double>);
    static_assert(decltype(f2)::min == 3.0);
    static_assert(decltype(f2)::max == 16.0);
}

TEST(bounded_member, add_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.1f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{2.0f};
    f2 += f1;

    ASSERT_FLOAT_EQ(*f2, 4.1f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == 2.0f);
    static_assert(decltype(f2)::max == 6.0f);
}

TEST(bounded_member, add_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{10.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{2.0f};
    ASSERT_THROW((f2 += f1), ufps::Exception);
}

TEST(bounded_member, add_assign_scalar)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.1f};
    f += 2.0f;

    ASSERT_FLOAT_EQ(*f, 4.1f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_member, add_assign_scalar_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{10.0f};
    ASSERT_THROW((f += 100.0f), ufps::Exception);
}

TEST(bounded_number, add_assign_smaller_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.2f};
    f += ufps::BoundedNumber<std::uint16_t, 2, 6>{3};

    ASSERT_FLOAT_EQ(*f, 5.2f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, add_assign_smaller_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{8.2f};
    ASSERT_THROW((f += ufps::BoundedNumber<std::uint16_t, 2, 6>{3}), ufps::Exception);
}

TEST(bounded_number, add_assign_larger_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.2f};
    f += ufps::BoundedDouble<>{3.0};

    ASSERT_FLOAT_EQ(*f, 5.2f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, add_assign_larger_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.2f};
    ASSERT_THROW((f += ufps::BoundedDouble<>{30}), ufps::Exception);
}

TEST(bounded_number, constexpr_add)
{
    constexpr auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.0f};
    constexpr auto f2 = ufps::BoundedFloat<1.0f, 10.0f>{3.0f};

    constexpr auto f3 = f1 + f2;

    static_assert(*f3 == 5.0f);
    static_assert(decltype(f3)::min == 2.0f);
    static_assert(decltype(f3)::max == 20.0f);
}

TEST(bounded_number, sub_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{2.0f};
    auto f3 = f1 - f2;

    ASSERT_FLOAT_EQ(*f3, 3.5f);

    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == -5.0f);
    static_assert(decltype(f3)::max == 8.0f);
}

TEST(bounded_number, sub_scalar)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};

    auto f2 = f1 - 2.0f;
    ASSERT_FLOAT_EQ(*f2, 3.5f);
    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f2)::max == std::numeric_limits<float>::max());

    auto f3 = 2.0f - f1;
    ASSERT_FLOAT_EQ(*f3, -3.5f);
    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f3)::max == std::numeric_limits<float>::max());
}

TEST(bounded_number, sub_smaller_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    auto f2 = f1 - ufps::BoundedNumber<std::uint16_t, 2, 6>{3};

    ASSERT_FLOAT_EQ(*f2, 2.5f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == -5.0f);
    static_assert(decltype(f2)::max == 8.0f);
}

TEST(bounded_number, sub_larger_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    auto f2 = f1 - ufps::BoundedDouble<2.0, 6.0>{3.0};

    ASSERT_DOUBLE_EQ(*f2, 2.5);

    static_assert(std::same_as<decltype(f2)::type, double>);
    static_assert(decltype(f2)::min == -5.0);
    static_assert(decltype(f2)::max == 8.0);
}

TEST(bounded_number, sub_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{2.0f};
    f1 -= f2;

    ASSERT_FLOAT_EQ(*f1, 3.5f);

    static_assert(std::same_as<decltype(f1)::type, float>);
    static_assert(decltype(f1)::min == 1.0f);
    static_assert(decltype(f1)::max == 10.0f);
}

TEST(bounded_number, sub_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{2.5f};
    auto f2 = ufps::BoundedFloat<2.0f, 6.0f>{4.0f};
    ASSERT_THROW((f1 -= f2), ufps::Exception);
}

TEST(bounded_number, sub_assign_scalar)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    f -= 2.0f;

    ASSERT_FLOAT_EQ(*f, 3.5f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, sub_assign_scalar_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{5.0f};
    ASSERT_THROW((f -= 10.0f), ufps::Exception);
}

TEST(bounded_number, sub_assign_smaller_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    f -= ufps::BoundedNumber<std::uint16_t, 2, 6>{3};

    ASSERT_FLOAT_EQ(*f, 2.5f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, sub_assign_smaller_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{3.5f};
    ASSERT_THROW((f -= ufps::BoundedNumber<std::uint16_t, 2, 6>{5}), ufps::Exception);
}

TEST(bounded_number, sub_assign_larger_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    f -= ufps::BoundedDouble<>{3.0};

    ASSERT_FLOAT_EQ(*f, 2.5f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, sub_assign_larger_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{3.5f};
    ASSERT_THROW((f -= ufps::BoundedDouble<>{5.0}), ufps::Exception);
}

TEST(bounded_number, constexpr_sub)
{
    constexpr auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{5.5f};
    constexpr auto f2 = ufps::BoundedFloat<1.0f, 10.0f>{2.0f};

    constexpr auto f3 = f1 - f2;

    static_assert(*f3 == 3.5f);
    static_assert(decltype(f3)::min == -9.0f);
    static_assert(decltype(f3)::max == 9.0f);
}

TEST(bounded_number, mul_same_type)
{
    auto f1 = ufps::BoundedFloat<-5.0f, 5.0f>{-2.0f};
    auto f2 = ufps::BoundedFloat<-3.0f, 4.0f>{3.0f};
    auto f3 = f1 * f2;

    ASSERT_FLOAT_EQ(*f3, -6.0f);

    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == -20.0f);
    static_assert(decltype(f3)::max == 20.0f);
}

TEST(bounded_number, mul_scalar)
{
    auto f1 = ufps::BoundedFloat<-5.0f, 5.0f>{2.0f};

    auto f2 = f1 * -2.0f;
    ASSERT_FLOAT_EQ(*f2, -4.0f);
    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f2)::max == std::numeric_limits<float>::max());

    auto f3 = -2.0f * f1;
    ASSERT_FLOAT_EQ(*f3, -4.0f);
    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f3)::max == std::numeric_limits<float>::max());
}

TEST(bounded_number, mul_smaller_type)
{
    auto f1 = ufps::BoundedFloat<-10.0f, -2.0f>{-5.0f};
    auto f2 = f1 * ufps::BoundedNumber<std::uint16_t, 2, 6>{3};

    ASSERT_FLOAT_EQ(*f2, -15.0f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == -60.0f);
    static_assert(decltype(f2)::max == -4.0f);
}

TEST(bounded_number, mul_larger_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 5.0f>{2.0f};
    auto f2 = f1 * ufps::BoundedDouble<-4.0, -1.0>{-3.0};

    ASSERT_DOUBLE_EQ(*f2, -6.0);

    static_assert(std::same_as<decltype(f2)::type, double>);
    static_assert(decltype(f2)::min == -20.0);
    static_assert(decltype(f2)::max == -1.0);
}

TEST(bounded_number, mul_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<-10.0f, 10.0f>{2.0f};
    auto f2 = ufps::BoundedFloat<-2.0f, 3.0f>{-2.0f};
    f1 *= f2;

    ASSERT_FLOAT_EQ(*f1, -4.0f);

    static_assert(std::same_as<decltype(f1)::type, float>);
    static_assert(decltype(f1)::min == -10.0f);
    static_assert(decltype(f1)::max == 10.0f);
}

TEST(bounded_number, mul_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<-10.0f, 10.0f>{6.0f};
    auto f2 = ufps::BoundedFloat<-5.0f, -2.0f>{-2.0f};
    ASSERT_THROW((f1 *= f2), ufps::Exception);
}

TEST(bounded_number, mul_assign_scalar)
{
    auto f = ufps::BoundedFloat<-10.0f, 10.0f>{2.5f};
    f *= -2.0f;

    ASSERT_FLOAT_EQ(*f, -5.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == -10.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, mul_assign_scalar_out_of_range)
{
    auto f = ufps::BoundedFloat<-10.0f, 10.0f>{-6.0f};
    ASSERT_THROW((f *= 2.0f), ufps::Exception);
}

TEST(bounded_number, mul_assign_smaller_type)
{
    auto f = ufps::BoundedFloat<-20.0f, 20.0f>{-2.5f};
    f *= ufps::BoundedNumber<std::uint16_t, 2, 6>{2};

    ASSERT_FLOAT_EQ(*f, -5.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == -20.0f);
    static_assert(decltype(f)::max == 20.0f);
}

TEST(bounded_number, mul_assign_smaller_type_out_of_range)
{
    auto f = ufps::BoundedFloat<-10.0f, 10.0f>{-6.0f};
    ASSERT_THROW((f *= ufps::BoundedNumber<std::uint16_t, 2, 6>{2}), ufps::Exception);
}

TEST(bounded_number, mul_assign_larger_type)
{
    auto f = ufps::BoundedFloat<-20.0f, 20.0f>{2.5f};
    f *= ufps::BoundedDouble<-4.0, -2.0>{-2.0};

    ASSERT_FLOAT_EQ(*f, -5.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == -20.0f);
    static_assert(decltype(f)::max == 20.0f);
}

TEST(bounded_number, mul_assign_larger_type_out_of_range)
{
    auto f = ufps::BoundedFloat<-10.0f, 10.0f>{4.0f};
    ASSERT_THROW((f *= ufps::BoundedDouble<-5.0, -3.0>{-3.0}), ufps::Exception);
}

TEST(bounded_number, constexpr_mul)
{
    constexpr auto f1 = ufps::BoundedFloat<-5.0f, -1.0f>{-2.0f};
    constexpr auto f2 = ufps::BoundedFloat<-4.0f, 3.0f>{-2.0f};

    constexpr auto f3 = f1 * f2;

    static_assert(*f3 == 4.0f);

    static_assert(decltype(f3)::min == -15.0f);
    static_assert(decltype(f3)::max == 20.0f);
}

TEST(bounded_number, div_same_type)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 5.0f>{4.0f};
    auto f3 = f1 / f2;

    ASSERT_FLOAT_EQ(*f3, 2.0f);

    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == 0.8f);
    static_assert(decltype(f3)::max == 5.0f);
}

TEST(bounded_number, div_negative_bounds)
{
    auto f1 = ufps::BoundedFloat<-10.0f, -4.0f>{-8.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 4.0f>{2.0f};
    auto f3 = f1 / f2;

    ASSERT_FLOAT_EQ(*f3, -4.0f);

    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == -5.0f);
    static_assert(decltype(f3)::max == -1.0f);
}

TEST(bounded_number, div_by_zero_throws_at_runtime)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};

    auto f2 = ufps::BoundedFloat<-2.0f, 5.0f>{0.0f};

    ASSERT_THROW((f1 / f2), ufps::Exception);
}

TEST(bounded_number, div_scalar)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};

    auto f2 = f1 / 2.0f;
    ASSERT_FLOAT_EQ(*f2, 4.0f);
    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f2)::max == std::numeric_limits<float>::max());

    auto f3 = 8.0f / f1;
    ASSERT_FLOAT_EQ(*f3, 1.0f);
    static_assert(std::same_as<decltype(f3)::type, float>);
    static_assert(decltype(f3)::min == std::numeric_limits<float>::lowest());
    static_assert(decltype(f3)::max == std::numeric_limits<float>::max());
}

TEST(bounded_number, div_scalar_by_zero_throws)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};
    ASSERT_THROW((f1 / 0.0f), ufps::Exception);
}

TEST(bounded_number, div_smaller_type)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};
    auto f2 = f1 / ufps::BoundedNumber<std::uint16_t, 2, 5>{4};

    ASSERT_FLOAT_EQ(*f2, 2.0f);

    static_assert(std::same_as<decltype(f2)::type, float>);
    static_assert(decltype(f2)::min == 0.8f);
    static_assert(decltype(f2)::max == 5.0f);
}

TEST(bounded_number, div_larger_type)
{
    auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};
    auto f2 = f1 / ufps::BoundedDouble<2.0, 5.0>{4.0};

    ASSERT_DOUBLE_EQ(*f2, 2.0);

    static_assert(std::same_as<decltype(f2)::type, double>);
    static_assert(decltype(f2)::min == 0.8);
    static_assert(decltype(f2)::max == 5.0);
}

TEST(bounded_number, div_assign_same_type)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    auto f2 = ufps::BoundedFloat<2.0f, 5.0f>{4.0f};
    f1 /= f2;

    ASSERT_FLOAT_EQ(*f1, 2.0f);

    static_assert(std::same_as<decltype(f1)::type, float>);
    static_assert(decltype(f1)::min == 1.0f);
    static_assert(decltype(f1)::max == 10.0f);
}

TEST(bounded_number, div_assign_same_type_out_of_range)
{
    auto f1 = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    auto f2 = ufps::BoundedFloat<0.2f, 0.8f>{0.5f};
    ASSERT_THROW((f1 /= f2), ufps::Exception);
}

TEST(bounded_number, div_assign_scalar)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    f /= 4.0f;

    ASSERT_FLOAT_EQ(*f, 2.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, div_assign_scalar_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    ASSERT_THROW((f /= 0.5f), ufps::Exception);
}

TEST(bounded_number, div_assign_smaller_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    f /= ufps::BoundedNumber<std::uint16_t, 2, 5>{4};

    ASSERT_FLOAT_EQ(*f, 2.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, div_assign_smaller_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.0f};
    ASSERT_THROW((f /= ufps::BoundedNumber<std::uint16_t, 2, 6>{5}), ufps::Exception);
}

TEST(bounded_number, div_assign_larger_type)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{8.0f};
    f /= ufps::BoundedDouble<2.0, 5.0>{4.0};

    ASSERT_FLOAT_EQ(*f, 2.0f);

    static_assert(std::same_as<decltype(f)::type, float>);
    static_assert(decltype(f)::min == 1.0f);
    static_assert(decltype(f)::max == 10.0f);
}

TEST(bounded_number, div_assign_larger_type_out_of_range)
{
    auto f = ufps::BoundedFloat<1.0f, 10.0f>{2.0f};
    ASSERT_THROW((f /= ufps::BoundedDouble<2.0, 8.0>{5.0}), ufps::Exception);
}

TEST(bounded_number, constexpr_div)
{
    constexpr auto f1 = ufps::BoundedFloat<4.0f, 10.0f>{8.0f};
    constexpr auto f2 = ufps::BoundedFloat<2.0f, 5.0f>{4.0f};

    constexpr auto f3 = f1 / f2;

    static_assert(*f3 == 2.0f);
    static_assert(decltype(f3)::min == 0.8f);
    static_assert(decltype(f3)::max == 5.0f);
}
