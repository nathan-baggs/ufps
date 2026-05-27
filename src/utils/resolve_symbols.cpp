#include "utils/resolve_symbols.h"

#include <cstddef>
#include <errhandlingapi.h>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dbghelp.h>

#include "concurrency/lock.h"
#include "utils/auto_release.h"
#include "utils/error.h"

namespace
{

static auto g_sym_handle = []
{
    const auto current_process_handle = ::GetCurrentProcess();
    auto h = ::HANDLE{};

    ufps::ensure(
        ::DuplicateHandle(
            current_process_handle,
            current_process_handle,
            current_process_handle,
            &h,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS) != 0,
        "failed to duplicate handle: {}",
        ::GetLastError());

    auto auto_handle = ufps::AutoRelease<::HANDLE, nullptr>{
        h,
        [](auto h)
        {
            ::SymCleanup(h);
            ::CloseHandle(h);
        }};

    ufps::ensure(
        ::SymInitialize(h, nullptr, TRUE) == TRUE, "failed to initialise symbol handler: {}", ::GetLastError());

    ::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);

    return auto_handle;
}();

static auto g_lock = ufps::Lock{};

}

namespace ufps
{

auto resolve_symbols(std::span<void *> call_stack) -> std::vector<std::string>
{
    const auto lck = std::scoped_lock{g_lock};

    return call_stack |
           std::views::transform(
               [](auto e) -> std::string
               {
                   static constexpr auto max_string_length = 512u;

                   auto symbol_buffer = std::array<std::byte, sizeof(::SYMBOL_INFO) + max_string_length>{};
                   auto *symbol = new (symbol_buffer.data())::SYMBOL_INFO{};

                   symbol->SizeOfStruct = sizeof(::SYMBOL_INFO);
                   symbol->MaxNameLen = max_string_length + 1u;

                   auto displacement = ::DWORD64{};

                   auto res = ::SymFromAddr(g_sym_handle, reinterpret_cast<::DWORD64>(e), &displacement, symbol);

                   return res == TRUE ? std::string(
                                            symbol_buffer.data() + sizeof(::SYMBOL_INFO) - 1u,
                                            symbol_buffer.data() + sizeof(::SYMBOL_INFO) - 1u + symbol->NameLen)
                                      : std::string{};
               }) |
           std::ranges::to<std::vector>();
}

}
