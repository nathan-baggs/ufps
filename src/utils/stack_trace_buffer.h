#pragma once

#include <inplace_vector>

namespace ufps
{

using StackTraceBuffer = std::inplace_vector<void *, 100zu>;

}
