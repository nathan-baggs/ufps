#pragma once

#include <span>
#include <string>
#include <vector>

namespace ufps
{

auto resolve_symbols(std::span<void *> call_stack) -> std::vector<std::string>;

}
