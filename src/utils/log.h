#pragma once

#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <source_location>
#include <string_view>

#include "config.h"

namespace ufps::log
{

namespace impl
{

inline static auto log_file = []
{
    auto f = std::ofstream{"log", std::ios::app};
    if (!f)
    {
        std::terminate();
    }

    return f;
}();

}

enum class Level
{
    DEBUG,
    INFO,
    WARN,
    ERR
};

inline auto g_force_log = false;

template <Level L, class... Args>
struct Print
{
    Print(std::format_string<Args...> msg, Args &&...args, std::source_location loc = std::source_location::current())
    {
        if (!(g_force_log || config::logging_enabled))
        {
            return;
        }

        auto c = '?';
        if constexpr (L == Level::DEBUG)
        {
            c = 'D';
        }
        else if constexpr (L == Level::INFO)
        {
            c = 'I';
        }
        else if constexpr (L == Level::WARN)
        {
            c = 'W';
        }
        else if constexpr (L == Level::ERR)
        {
            c = 'E';
        }

        const auto path = std::filesystem::path{loc.file_name()};

        const auto log_line = std::format(
            "[{}] {}:{} {}", c, path.filename().string(), loc.line(), std::format(msg, std::forward<Args>(args)...));

        std::println("{}", log_line);

        if constexpr (config::log_to_file)
        {
            impl::log_file << log_line << std::endl;
        }
    }
};

template <Level L = {}, class... Args>
Print(std::format_string<Args...>, Args &&...) -> Print<L, Args...>;

template <class... Args>
using debug = Print<Level::DEBUG, Args...>;

template <class... Args>
using info = Print<Level::INFO, Args...>;

template <class... Args>
using warn = Print<Level::WARN, Args...>;

template <class... Args>
using error = Print<Level::ERR, Args...>;

}
