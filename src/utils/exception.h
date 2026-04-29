#pragma once

#include <format>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace ufps
{

class Exception : public std::runtime_error
{
  public:
    template <class... Args>
    constexpr Exception(std::format_string<Args...> msg, Args &&...args);

    constexpr auto to_string(this auto &&self) -> std::string;

    constexpr auto what() const noexcept -> const char * override;

  private:
    std::string what_;
};

template <class... Args>
constexpr Exception::Exception(std::format_string<Args...> msg, Args &&...args)
    : std::runtime_error{std::format(msg, std::forward<Args>(args)...)}
    , what_{std::format("{}\n{}", std::runtime_error::what(), std::stacktrace::current(1)).c_str()}
{
}

constexpr auto Exception::to_string(this auto &&self) -> std::string
{
    return self.what_;
}

constexpr auto Exception::what() const noexcept -> const char *
{
    return what_.c_str();
}

}
