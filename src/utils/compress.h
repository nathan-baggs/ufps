#pragma once

#include <expected>
#include <string>

#include "utils/data_buffer.h"

namespace ufps
{

auto compress(DataBufferView data) -> std::expected<DataBuffer, std::string>;

}
