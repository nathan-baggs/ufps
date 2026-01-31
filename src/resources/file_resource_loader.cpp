#include "resources/file_resource_loader.h"

#include <cstddef>
#include <filesystem>
#include <vector>

#include <handleapi.h>
#include <windows.h>

#include "utils/auto_release.h"
#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/formatter.h"
#include "utils/log.h"

namespace
{

auto init(const std::filesystem::path &path)
{
    auto handle = ufps::AutoRelease<HANDLE, nullptr>{
        ::CreateFileA(path.string().c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr),
        ::CloseHandle};
    ufps::ensure(
        handle.get() != INVALID_HANDLE_VALUE, "failed to open file: {} error: {}", path.string(), ::GetLastError());

    auto mapping = ufps::AutoRelease<HANDLE, nullptr>{
        ::CreateFileMappingA(handle, nullptr, PAGE_READONLY, 0, 0, nullptr), ::CloseHandle};
    ufps::ensure(mapping, "failed to map file: {} error: {}", path.string(), ::GetLastError());

    auto map_view = std::unique_ptr<void, decltype(&::UnmapViewOfFile)>{
        ::MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0), ::UnmapViewOfFile};
    ufps::ensure(map_view, "failed to get map view: {} error: {}", path.string(), ::GetLastError());

    return std::make_tuple(std::move(handle), std::move(mapping), std::move(map_view));
}

template <class T>
auto load(const std::filesystem::path &path)
{
    static_assert(sizeof(typename T::value_type) == 1);

    const auto &[handle, mapping, map_view] = init(path);
    const auto size = ::GetFileSize(handle, nullptr);
    const auto *ptr = reinterpret_cast<T::value_type *>(map_view.get());

    ufps::log::info("loaded resource: {} ({})", path.string(), size);

    return T{ptr, ptr + size};
}
}

namespace ufps
{
FileResourceLoader::FileResourceLoader(const std::vector<std::filesystem::path> &roots)
    : roots_{roots}
{
}

auto FileResourceLoader::load_string(std::string_view name) -> std::string
{
    for (const auto &root : roots_)
    {
        const auto path = root / name;
        if (std::filesystem::exists(path))
        {
            return load<std::string>(path);
        }
    }

    throw Exception("cannot find {}", name);
}

auto FileResourceLoader::load_data_buffer(std::string_view name) -> DataBuffer
{
    for (const auto &root : roots_)
    {
        const auto path = root / name;
        if (std::filesystem::exists(path))
        {
            return load<DataBuffer>(path);
        }
    }

    throw Exception("cannot find {}", name);
}
}
