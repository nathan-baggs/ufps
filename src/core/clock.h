#pragma once

#include <chrono>

namespace ufps
{

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::microseconds;

}
