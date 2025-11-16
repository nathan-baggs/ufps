#pragma once

#include <concepts>
#include <format>
#include <string>

namespace ufps
{

template <class T>
concept HasToStringMember = requires(T a) {
    { a.to_string() } -> std::convertible_to<std::string>;
};

template <class T>
concept HasToStringFree = requires(T a) {
    { to_string(a) } -> std::convertible_to<std::string>;
};

namespace util
{

struct ToStringCPO
{
    template <HasToStringMember T>
    auto operator()(T &&obj) const -> std::string
    {
        return obj.to_string();
    }

    template <class T>
        requires(!HasToStringMember<T> && HasToStringFree<T>)
    auto operator()(T &&obj) const -> std::string
    {
        return to_string(obj);
    }
};

inline constexpr auto to_string = ToStringCPO{};

}

template <class T>
struct Formatter
{
    constexpr auto parse(std::format_parse_context &ctx)
    {
        return std::ranges::begin(ctx);
    }

    auto format(const T &obj, std::format_context &ctx) const
    {
        return std::format_to(ctx.out(), "{}", util::to_string(obj));
    }
};
}

template <class T>
concept CanFormat = requires(T a) {
    { ufps::util::to_string(a) } -> std::convertible_to<std::string>;
};

template <CanFormat T>
struct std::formatter<T> : ufps::Formatter<T>
{
};
