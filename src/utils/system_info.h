#pragma once

#include <string>

namespace ufps
{

struct SystemInfo
{
    std::string os_version;
    std::string gpu_id;
    std::string gpu_driver;
    std::string system_memory;
};

auto system_info() -> SystemInfo;

auto to_string(const SystemInfo &info) -> std::string;

}
