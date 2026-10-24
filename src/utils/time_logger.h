#pragma once

#include <chrono>
#include <format>
#include <source_location>
#include <string>

#include "core/clock.h"
#include "utils/log.h"

using namespace std::literals;

namespace ufps
{

class TimeLogger
{
  public:
    TimeLogger(Duration threshold = 0ms, std::source_location location = std::source_location::current())
        : start_{Clock::now()}
        , location_{std::move(location)}
        , threshold_{threshold}
        , message_{}
    {
    }

    TimeLogger(
        std::string message,
        Duration threshold = 0ms,
        std::source_location location = std::source_location::current())
        : start_{Clock::now()}
        , location_{std::move(location)}
        , threshold_{threshold}
        , message_{std::move(message)}
    {
    }

    ~TimeLogger()
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(Clock::now() - start_);

        if (elapsed >= threshold_)
        {
            log::info(
                "{} {}({}) {} finished in: {}",
                location_.file_name(),
                location_.function_name(),
                location_.line(),
                message_,
                elapsed);
        }
    }

  private:
    TimePoint start_;
    std::source_location location_;
    Duration threshold_;
    std::string message_;
};

}
