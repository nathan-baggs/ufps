#include "utils/resolve_symbols.h"

#include <cstddef>
#include <errhandlingapi.h>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include <backtrace-supported.h>
#include <backtrace.h>
#include <cxxabi.h>

#include "concurrency/lock.h"
#include "utils/auto_release.h"
#include "utils/error.h"
#include "utils/exception.h"

namespace
{
auto error_callback(void *, const char *msg, int errnum) -> void
{
    ufps::drop_mic("backtrace error: {} {}", msg, errnum);
}

auto *g_state = []
{
    auto *state = ::backtrace_create_state(nullptr, BACKTRACE_SUPPORTS_THREADS, error_callback, nullptr);
    return state;
}();

auto syminfo_callback(void *data, std::uintptr_t pc, const char *symname, std::uintptr_t, std::uintptr_t) -> void
{
    auto *symbol = reinterpret_cast<std::string *>(data);

    if (symname)
    {
        *symbol = std::format("{:x} {} ??:0", pc, symname);
    }
    else
    {
        *symbol = std::format("{:x} ?? ??:0", pc);
    }
}

auto full_callback(void *data, std::uintptr_t pc, const char *filename, int lineno, const char *function) -> int
{
    auto *symbol = reinterpret_cast<std::string *>(data);
    if (function)
    {
        auto status = int{};
        const auto real_name = ::abi::__cxa_demangle(function, nullptr, nullptr, &status);
        if (real_name)
        {
            *symbol = std::format("{:x} {} {}:{}", pc, real_name, filename, lineno);
            std::free(real_name);
        }
        else
        {
            *symbol = std::format("{:x} {} {}:{}", pc, function, filename, lineno);
        }
    }
    else
    {
        ::backtrace_syminfo(g_state, pc, syminfo_callback, error_callback, data);
    }

    return 0;
}

auto g_lock = ufps::Lock{};

}

namespace ufps
{

auto resolve_symbols(std::span<void *> call_stack) -> std::vector<std::string>
{
    const auto lck = std::scoped_lock{g_lock};

    return call_stack |
           std::views::transform(
               [](auto e)
               {
                   auto symbol = std::string{};
                   ::backtrace_pcinfo(
                       g_state, reinterpret_cast<std::uintptr_t>(e), full_callback, error_callback, &symbol);
                   return symbol;
               }) |
           std::views::filter([](const auto &e) { return !e.contains("register_frame_ctor"); }) |
           std::ranges::to<std::vector>();
}
}
