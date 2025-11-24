#pragma once

#include <string>
#include <string_view>

namespace ufps
{

auto text_widen(std::string_view str) -> std::wstring;
auto text_narrow(std::wstring_view str) -> std::string;

}
