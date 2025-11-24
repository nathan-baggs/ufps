#include "utils/text_utils.h"

#include <string>
#include <string_view>

#include <windows.h>

#include "utils/error.h"

namespace ufps
{

auto text_widen(std::string_view str) -> std::wstring
{
    const auto num_wide_chars =
        ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, str.data(), static_cast<int>(str.size()), nullptr, 0);
    ensure(num_wide_chars != 0, "failed to get wstring size for converting: {}", str);

    auto wide_str = std::wstring(num_wide_chars, L'\0');

    ensure(
        ::MultiByteToWideChar(
            CP_UTF8,
            MB_PRECOMPOSED,
            str.data(),
            static_cast<int>(str.size()),
            wide_str.data(),
            static_cast<int>(wide_str.size())) == static_cast<int>(wide_str.size()),
        "failed to widen string: {}",
        str);

    return wide_str;
}

auto text_narrow(std::wstring_view str) -> std::string
{
    const auto num_multi_chars = ::WideCharToMultiByte(
        CP_UTF8, WC_NO_BEST_FIT_CHARS, str.data(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
    ensure(num_multi_chars != 0, "failed to get string size for converting");

    auto narrow_str = std::string(num_multi_chars, '\0');

    ensure(
        ::WideCharToMultiByte(
            CP_UTF8,
            WC_NO_BEST_FIT_CHARS,
            str.data(),
            static_cast<int>(str.size()),
            narrow_str.data(),
            static_cast<int>(narrow_str.size()),
            nullptr,
            nullptr) == static_cast<int>(narrow_str.size()),
        "failed to narrow string");

    return narrow_str;
}

}
