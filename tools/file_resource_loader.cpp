#include "resources/file_resource_loader.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "utils/auto_release.h"
#include "utils/data_buffer.h"
#include "utils/error.h"
#include "utils/formatter.h"
#include "utils/log.h"
#include "utils/time_logger.h"

using namespace std::literals;

namespace
{

auto to_native_path(std::string_view name) -> std::filesystem::path
{
    auto native_name = std::string{name};
    std::ranges::replace(native_name, '\\', '/');
    return std::filesystem::path{native_name};
}

auto init(const std::filesystem::path &path)
{
    auto handle = ufps::AutoRelease<int, -1>{::open(path.c_str(), O_RDONLY, 0), ::close};
    ufps::ensure(handle.get() != -1, "failed to open file: {} error: {}", path.native_encoded_string(), errno);

    const auto size = std::filesystem::file_size(path);
    auto *address = ::mmap(nullptr, size, PROT_READ, MAP_PRIVATE, handle.get(), 0);
    ufps::ensure(address != MAP_FAILED, "failed to mmap: {} error: {}", path.native_encoded_string(), errno);

    auto unmap = [size](void *ptr) { ::munmap(ptr, size); };
    auto map = std::unique_ptr<void, decltype(unmap)>{address, unmap};

    return std::make_tuple(std::move(handle), std::move(map));
}

template <class T>
auto load(const std::filesystem::path &path)
{
    const auto time_logger = ufps::TimeLogger{std::format("loading: {}", path), 1s};

    static_assert(sizeof(typename T::value_type) == 1);

    const auto &[handle, map] = init(path);
    const auto size = std::filesystem::file_size(path);
    const auto *ptr = reinterpret_cast<T::value_type *>(map.get());

    return T{ptr, ptr + size};
}
}

namespace ufps
{
FileResourceLoader::FileResourceLoader(const std::vector<std::filesystem::path> &roots)
    : roots_{roots}
{
    for (const auto &root : roots_)
    {
        if (!std::filesystem::exists(root))
        {
            throw Exception("resource root does not exist: {}", root.native_encoded_string());
        }
    }
}

auto FileResourceLoader::load_string(std::string_view name) -> std::string
{
    const auto relative_path = to_native_path(name);
    for (const auto &root : roots_)
    {
        const auto path = root / relative_path;
        if (std::filesystem::exists(path))
        {
            return load<std::string>(path);
        }
    }

    throw Exception("cannot find {}", name);
}

auto FileResourceLoader::load_data_buffer(std::string_view name) -> DataBuffer
{
    const auto relative_path = to_native_path(name);
    for (const auto &root : roots_)
    {
        const auto path = root / relative_path;
        if (std::filesystem::exists(path))
        {
            return load<DataBuffer>(path);
        }
    }

    throw Exception("cannot find {}", name);
}

auto FileResourceLoader::resources(std::string_view type) -> std::vector<std::string>
{
    return roots_ | std::views::filter([&](const auto &e) { return std::filesystem::is_directory(e / type); }) |
           std::views::transform(
               [&](const auto &e)
               {
                   const auto dir_iter = std::filesystem::directory_iterator{e / type};
                   return dir_iter | std::views::filter([&](const auto &e) { return e.is_regular_file(); }) |
                          std::views::transform(
                              [&](const auto &e)
                              { return std::format("{}\\{}", type, e.path().filename().native_encoded_string()); }) |
                          std::ranges::to<std::vector>();
               }) |
           std::views::join | std::ranges::to<std::vector>();
}
}
