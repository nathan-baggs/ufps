#include <cstddef>
#include <stdexcept>

#include <windows.h>

namespace
{

auto heap() -> ::HANDLE
{
    static auto h = []
    {
        const auto h = ::HeapCreate(0, 0, 0);
        if (h == INVALID_HANDLE_VALUE)
        {
            throw std::bad_alloc{};
        }
        return h;
    }();

    return h;
}

}

auto operator new(std::size_t count) -> void *
{
    auto *ptr = ::HeapAlloc(heap(), 0, count);
    if (ptr == nullptr)
    {
        throw std::bad_alloc{};
    }

    return ptr;
}

auto operator new[](std::size_t count) -> void *
{
    return ::operator new(count);
}

auto operator delete(void *ptr) noexcept -> void
{
    ::HeapFree(heap(), 0, ptr);
}

auto operator delete[](void *ptr) noexcept -> void
{
    ::HeapFree(heap(), 0, ptr);
}

auto operator delete(void *ptr, std::size_t) noexcept -> void
{
    ::operator delete(ptr);
}

auto operator delete[](void *ptr, std::size_t) noexcept -> void
{
    ::operator delete(ptr);
}

namespace ufps
{
auto allocation_size(void *ptr) -> std::size_t;
auto allocation_size(void *ptr) -> std::size_t
{
    return ::HeapSize(heap(), 0, ptr);
}
}
