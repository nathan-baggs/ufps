#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ufps
{

using DataBuffer = std::vector<std::byte>;
using DataBufferView = std::span<const std::byte>;

}
