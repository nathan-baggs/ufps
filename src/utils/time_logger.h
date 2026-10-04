#pragma once

#include <source_location>

#include "core/clock.h"
#include "utils/log.h"

namespace ufps
{

class TimeLogger
{
  public:
    TimeLogger(std::source_location location = std::source_location::current())
        : start_{Clock::now()}
        , location_{std::move(location)}
    {
    }

    ~TimeLogger()
    {
        log::info(
            "{} {}({}) finished in: {}",
            location_.file_name(),
            location_.function_name(),
            location_.line(),
            std::chrono::duration_cast<std::chrono::duration<float>>(Clock::now() - start_));
    }

  private:
    TimePoint start_;
    std::source_location location_;
};

}
