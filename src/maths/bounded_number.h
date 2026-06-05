#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "utils/error.h"

namespace ufps
{

template <class T, T Min = std::numeric_limits<T>::lowest(), T Max = std::numeric_limits<T>::max()>
class BoundedNumber
{
  public:
    inline static constexpr auto min = Min;
    inline static constexpr auto max = Max;
    using type = std::remove_cv_t<T>;

    constexpr BoundedNumber();

    constexpr BoundedNumber(T value);

    template <class U, U OtherMin, U OtherMax>
    constexpr BoundedNumber(const BoundedNumber<U, OtherMin, OtherMax> &other);

    template <class U, U OtherMin, U OtherMax>
    constexpr auto operator=(const BoundedNumber<U, OtherMin, OtherMax> &other) -> BoundedNumber<T, Min, Max> &;

    template <class U, U OtherMin, U OtherMax>
    constexpr BoundedNumber(BoundedNumber<U, OtherMin, OtherMax> &&other);

    template <class U, U OtherMin, U OtherMax>
    constexpr auto operator=(BoundedNumber<U, OtherMin, OtherMax> &&other) -> BoundedNumber<T, Min, Max> &;

    template <class U, U OtherMin, U OtherMax>
    constexpr auto operator<=>(const BoundedNumber<U, OtherMin, OtherMax> &other) const;

    template <class U, U OtherMin, U OtherMax>
    constexpr auto operator==(const BoundedNumber<U, OtherMin, OtherMax> &other) const;

    constexpr auto value() const -> T;

    constexpr auto operator*() const -> T;

    constexpr operator T() const;

    constexpr auto operator&(this auto &&self) -> T *;

    constexpr auto operator-() const -> BoundedNumber<T, static_cast<T>(-Max), static_cast<T>(-Min)>
        requires std::is_signed_v<T>;

    constexpr auto operator+() const;

  private:
    T value_;
};

template <class T, T Min, T Max>
constexpr BoundedNumber<T, Min, Max>::BoundedNumber()
    : BoundedNumber(T{})
{
}

template <class T, T Min, T Max>
constexpr BoundedNumber<T, Min, Max>::BoundedNumber(T value)
    : value_{value}
{
    ensure(value_ >= Min && value_ <= Max, "{} not in range ({}, {})", value_, Min, Max);
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr BoundedNumber<T, Min, Max>::BoundedNumber(const BoundedNumber<U, OtherMin, OtherMax> &other)
    : BoundedNumber{*other}
{
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr auto BoundedNumber<T, Min, Max>::operator=(const BoundedNumber<U, OtherMin, OtherMax> &other)
    -> BoundedNumber<T, Min, Max> &
{
    *this = BoundedNumber<T, Min, Max>{*other};
    return *this;
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr BoundedNumber<T, Min, Max>::BoundedNumber(BoundedNumber<U, OtherMin, OtherMax> &&other)
    : BoundedNumber{*other}
{
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr auto BoundedNumber<T, Min, Max>::operator=(BoundedNumber<U, OtherMin, OtherMax> &&other)
    -> BoundedNumber<T, Min, Max> &
{
    *this = BoundedNumber<T, Min, Max>{*other};
    return *this;
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr auto BoundedNumber<T, Min, Max>::operator<=>(const BoundedNumber<U, OtherMin, OtherMax> &other) const
{
    return value_ <=> *other;
}

template <class T, T Min, T Max>
template <class U, U OtherMin, U OtherMax>
constexpr auto BoundedNumber<T, Min, Max>::operator==(const BoundedNumber<U, OtherMin, OtherMax> &other) const
{
    return value_ == *other;
}

template <class T, T Min, T Max>
constexpr auto BoundedNumber<T, Min, Max>::value() const -> T
{
    return value_;
}

template <class T, T Min, T Max>
constexpr auto BoundedNumber<T, Min, Max>::operator*() const -> T
{
    return value_;
}

template <class T, T Min, T Max>
constexpr auto BoundedNumber<T, Min, Max>::operator&(this auto &&self) -> T *
{
    return std::addressof(self.value_);
}

template <class T, T Min, T Max>
constexpr auto BoundedNumber<T, Min, Max>::operator-() const
    -> BoundedNumber<T, static_cast<T>(-Max), static_cast<T>(-Min)>
    requires std::is_signed_v<T>
{
    return BoundedNumber<T, -Max, -Min>(-value_);
}

template <class T, T Min, T Max>
constexpr auto BoundedNumber<T, Min, Max>::operator+() const
{
    const auto new_value = +value_;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType, static_cast<NewType>(Min), static_cast<NewType>(Max)>{new_value};
}

template <class T, T Min, T Max>
constexpr BoundedNumber<T, Min, Max>::operator T() const
{
    return value_;
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto operator+(const BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 + *num2;
    using NewType = decltype(new_value);

    return BoundedNumber<
        NewType,
        static_cast<NewType>(TMin) + static_cast<NewType>(UMin),
        static_cast<NewType>(TMax) + static_cast<NewType>(UMax)>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator+(const BoundedNumber<T, TMin, TMax> &num, U u)
{
    const auto new_value = *num + u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator+(U u, const BoundedNumber<T, TMin, TMax> &num)
{
    return BoundedNumber<U>{u} + num;
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto &operator+=(BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 + *num2;

    ensure(
        new_value >= TMin && new_value <= TMax,
        "{} + {} = {}, out of range ({}, {})",
        *num1,
        *num2,
        new_value,
        TMin,
        TMax);

    num1 = new_value;

    return num1;
}

template <class T, T TMin, T TMax, class U>
constexpr auto &operator+=(BoundedNumber<T, TMin, TMax> &num, U u)
{
    return num += BoundedNumber<U>{u};
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto operator-(const BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 - *num2;
    using NewType = decltype(new_value);

    return BoundedNumber<
        NewType,
        static_cast<NewType>(TMin) - static_cast<NewType>(UMax),
        static_cast<NewType>(TMax) - static_cast<NewType>(UMin)>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator-(const BoundedNumber<T, TMin, TMax> &num, U u)
{
    const auto new_value = *num - u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator-(U u, const BoundedNumber<T, TMin, TMax> &num)
{
    return BoundedNumber<U>{u} - num;
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto &operator-=(BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 - *num2;

    ensure(
        new_value >= TMin && new_value <= TMax,
        "{} - {} = {}, out of range ({}, {})",
        *num1,
        *num2,
        new_value,
        TMin,
        TMax);

    num1 = new_value;

    return num1;
}

template <class T, T TMin, T TMax, class U>
constexpr auto &operator-=(BoundedNumber<T, TMin, TMax> &num, U u)
{
    return num -= BoundedNumber<U>{u};
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto operator*(const BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 * *num2;
    using NewType = decltype(new_value);

    return BoundedNumber<
        NewType,
        std::min(
            {static_cast<NewType>(TMin) * static_cast<NewType>(UMin),
             static_cast<NewType>(TMin) * static_cast<NewType>(UMax),
             static_cast<NewType>(TMax) * static_cast<NewType>(UMin),
             static_cast<NewType>(TMax) * static_cast<NewType>(UMax)}),
        std::max(
            {static_cast<NewType>(TMin) * static_cast<NewType>(UMin),
             static_cast<NewType>(TMin) * static_cast<NewType>(UMax),
             static_cast<NewType>(TMax) * static_cast<NewType>(UMin),
             static_cast<NewType>(TMax) * static_cast<NewType>(UMax)})>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator*(const BoundedNumber<T, TMin, TMax> &num, U u)
{
    const auto new_value = *num * u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType, std::numeric_limits<NewType>::lowest(), std::numeric_limits<NewType>::max()>{
        new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator*(U u, const BoundedNumber<T, TMin, TMax> &num)
{
    const auto new_value = *num * u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType, std::numeric_limits<NewType>::lowest(), std::numeric_limits<NewType>::max()>{
        new_value};
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto &operator*=(BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 * *num2;

    ensure(
        new_value >= TMin && new_value <= TMax,
        "{} * {} = {}, out of range ({}, {})",
        *num1,
        *num2,
        new_value,
        TMin,
        TMax);

    num1 = new_value;

    return num1;
}

template <class T, T TMin, T TMax, class U>
constexpr auto &operator/=(BoundedNumber<T, TMin, TMax> &num, U u)
{
    return num /= BoundedNumber<U>{u};
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto operator/(const BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 / *num2;
    using NewType = decltype(new_value);

    return BoundedNumber<
        NewType,
        std::min(
            {static_cast<NewType>(TMin) / static_cast<NewType>(UMin),
             static_cast<NewType>(TMin) / static_cast<NewType>(UMax),
             static_cast<NewType>(TMax) / static_cast<NewType>(UMin),
             static_cast<NewType>(TMax) / static_cast<NewType>(UMax)}),
        std::max(
            {static_cast<NewType>(TMin) / static_cast<NewType>(UMin),
             static_cast<NewType>(TMin) / static_cast<NewType>(UMax),
             static_cast<NewType>(TMax) / static_cast<NewType>(UMin),
             static_cast<NewType>(TMax) / static_cast<NewType>(UMax)})>{new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator/(const BoundedNumber<T, TMin, TMax> &num, U u)
{
    const auto new_value = *num / u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType, std::numeric_limits<NewType>::lowest(), std::numeric_limits<NewType>::max()>{
        new_value};
}

template <class T, T TMin, T TMax, class U>
constexpr auto operator/(U u, const BoundedNumber<T, TMin, TMax> &num)
{
    const auto new_value = *num / u;
    using NewType = decltype(new_value);

    return BoundedNumber<NewType, std::numeric_limits<NewType>::lowest(), std::numeric_limits<NewType>::max()>{
        new_value};
}

template <class T, T TMin, T TMax, class U, U UMin, U UMax>
constexpr auto &operator/=(BoundedNumber<T, TMin, TMax> &num1, const BoundedNumber<U, UMin, UMax> &num2)
{
    const auto new_value = *num1 / *num2;

    ensure(
        new_value >= TMin && new_value <= TMax,
        "{} / {} = {}, out of range ({}, {})",
        *num1,
        *num2,
        new_value,
        TMin,
        TMax);

    num1 = new_value;

    return num1;
}

template <class T, T TMin, T TMax, class U>
constexpr auto &operator*=(BoundedNumber<T, TMin, TMax> &num, U u)
{
    return num *= BoundedNumber<U>{u};
}

template <float Min = std::numeric_limits<float>::lowest(), float Max = std::numeric_limits<float>::max()>
using BoundedFloat = BoundedNumber<float, Min, Max>;
static_assert(sizeof(BoundedFloat<>) == sizeof(BoundedFloat<>::type));

template <double Min = std::numeric_limits<double>::lowest(), double Max = std::numeric_limits<double>::max()>
using BoundedDouble = BoundedNumber<double, Min, Max>;
static_assert(sizeof(BoundedDouble<>) == sizeof(BoundedDouble<>::type));

template <
    std::uint32_t Min = std::numeric_limits<std::uint32_t>::lowest(),
    std::uint32_t Max = std::numeric_limits<std::uint32_t>::max()>
using BoundedUint32 = BoundedNumber<std::uint32_t, Min, Max>;
static_assert(sizeof(BoundedUint32<>) == sizeof(BoundedUint32<>::type));

}
