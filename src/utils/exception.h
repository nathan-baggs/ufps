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
    Exception(std::format_string<Args...> msg, Args &&...args)
        : std::runtime_error{std::format(msg, std::forward<Args>(args)...)}
        , what_{std::format("{}\n{}", std::runtime_error::what(), std::stacktrace::current(1)).c_str()}
    {
    }

    auto to_string() const -> std::string
    {
        return what_;
    }

    auto what() const noexcept -> const char * override
    {
        return what_.c_str();
    }

  private:
    std::string what_;
};

}
