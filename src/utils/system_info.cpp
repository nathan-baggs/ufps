#include "system_info.h"

#include <format>
#include <ranges>
#include <string>

#include <comdef.h>
#include <wbemidl.h>
#include <windows.h>

#include "utils/exception.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/text_utils.h"
#include "utils/wmi.h"

namespace
{

auto os_version(ufps::Wmi &wmi) -> std::string
{
    try
    {
        const auto caption = wmi.query("SELECT * FROM Win32_OperatingSystem", "Caption") | std::views::join_with(' ') |
                             std::ranges::to<std::string>();
        const auto version = wmi.query("SELECT * FROM Win32_OperatingSystem", "Version") | std::views::join_with(' ') |
                             std::ranges::to<std::string>();

        return std::format("{} ({})", caption, version);
    }
    catch (ufps::Exception &e)
    {
        ufps::log::error("failed to get gpu_id: {}", e);
    }
    catch (...)
    {
        ufps::log::error("failed to get gpu_id");
    }

    return {};
}

auto gpu_id(const ufps::Wmi &wmi) -> std::string
{
    try
    {
        const auto props = wmi.query("SELECT * FROM Win32_VideoController", "Caption");
        return props | std::views::join_with(',') | std::ranges::to<std::string>();
    }
    catch (ufps::Exception &e)
    {
        ufps::log::error("failed to get gpu_id: {}", e);
    }
    catch (...)
    {
        ufps::log::error("failed to get gpu_id");
    }

    return {};
}

auto gpu_driver(const ufps::Wmi &wmi) -> std::string
{
    try
    {
        const auto props = wmi.query("SELECT * FROM Win32_VideoController", "DriverVersion");
        return props | std::views::join_with(' ') | std::ranges::to<std::string>();
    }
    catch (ufps::Exception &e)
    {
        ufps::log::error("failed to get gpu_driver: {}", e);
    }
    catch (...)
    {
        ufps::log::error("failed to get gpu_driver");
    }

    return {};
}

auto system_memory() -> std::string
{
    auto mem = ::ULONGLONG{};
    ::GetPhysicallyInstalledSystemMemory(&mem);

    return mem == 0 ? "" : std::format("{}", mem);
}

}

namespace ufps
{

auto system_info() -> SystemInfo
{
    auto wmi = Wmi{};
    return {
        .os_version = os_version(wmi),
        .gpu_id = gpu_id(wmi),
        .gpu_driver = gpu_driver(wmi),
        .system_memory = system_memory()};
}

auto to_string(const SystemInfo &info) -> std::string
{
    const auto get_or_empty = [](const auto &s) { return s.empty() ? "<unknown>" : s; };

    return std::format(
        "os_version: {}\ngpu_id: {}\ngpu_driver: {}\nsystem_memory: {}",
        get_or_empty(info.os_version),
        get_or_empty(info.gpu_id),
        get_or_empty(info.gpu_driver),
        get_or_empty(info.system_memory));
}

};
