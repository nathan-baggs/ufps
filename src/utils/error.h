#pragma once

#include <format>
#include <memory>
#include <stacktrace>
#include <utility>

#include "exception.h"
#include "log.h"
#include "utils/auto_release.h"

namespace ufps
{

template <class... Args>
constexpr auto expect(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
{
    if (!predicate)
    {
        log::error("{}", std::format(msg, std::forward<Args>(args)...));
        log::error("{}", std::stacktrace::current(1));
        std::terminate();
        std::unreachable();
    }
}

template <class T, class... Args>
constexpr auto expect(std::unique_ptr<T> &obj, std::format_string<Args...>(msg), Args &&...args) -> void
{
    expect(!!obj, msg, std::forward<Args>(args)...);
}

template <class... Args>
auto ensure(bool predicate, std::format_string<Args...> msg, Args &&...args) -> void
{
    if (!predicate)
    {
        throw Exception(msg, std::forward<Args>(args)...);
    }
}

template <class T, T Invalid, class... Args>
auto ensure(AutoRelease<T, Invalid> &obj, std::format_string<Args...> msg, Args &&...args) -> void
{
    ensure(!!obj, msg, std::forward<Args>(args)...);
}

template <class T, class D, class... Args>
auto ensure(std::unique_ptr<T, D> &obj, std::format_string<Args...> msg, Args &&...args) -> void
{
    ensure(!!obj, msg, std::forward<Args>(args)...);
}

}
