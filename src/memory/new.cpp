#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>

#include <windows.h>
#include <winnt.h>

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

auto operator new(std::size_t count, std::align_val_t al) -> void *
{
    const auto alignment = static_cast<std::size_t>(al);
    if (alignment <= MEMORY_ALLOCATION_ALIGNMENT)
    {
        return ::operator new(count);
    }

    const auto new_size = count + alignment + sizeof(void *);

    auto *ptr = ::operator new(new_size);
    auto size = new_size;

    auto *offset_ptr = reinterpret_cast<void *>(reinterpret_cast<char *>(ptr) + alignment);

    auto *aligned_ptr = std::align(alignment, count, offset_ptr, size);
    if (aligned_ptr == nullptr)
    {
        throw std::bad_alloc{};
    }

    *(reinterpret_cast<void **>(aligned_ptr) - 1) = ptr;

    return aligned_ptr;
}

auto operator new[](std::size_t count, std::align_val_t al) -> void *
{
    return ::operator new(count, al);
}

auto operator delete(void *ptr) noexcept -> void
{
    ::HeapFree(heap(), 0, ptr);
}

auto operator delete[](void *ptr) noexcept -> void
{
    ::operator delete(ptr);
}

auto operator delete(void *ptr, std::size_t) noexcept -> void
{
    ::operator delete(ptr);
}

auto operator delete[](void *ptr, std::size_t) noexcept -> void
{
    ::operator delete(ptr);
}

auto operator delete(void *ptr, std::align_val_t al) noexcept -> void
{
    const auto alignment = static_cast<std::size_t>(al);
    if (alignment <= MEMORY_ALLOCATION_ALIGNMENT)
    {
        ::operator delete(ptr);
        return;
    }

    auto *orig_ptr = *(reinterpret_cast<void **>(ptr) - 1);
    ::operator delete(orig_ptr);
}

auto operator delete(void *ptr, std::size_t, std::align_val_t al) noexcept -> void
{
    ::operator delete(ptr, al);
}

auto operator delete[](void *ptr, std::align_val_t al) noexcept -> void
{
    ::operator delete(ptr, al);
}

auto operator delete[](void *ptr, std::size_t, std::align_val_t al) noexcept -> void
{
    ::operator delete(ptr, al);
}

namespace ufps
{
auto allocation_size(void *ptr) -> std::size_t;
auto allocation_size(void *ptr) -> std::size_t
{
    return ::HeapSize(heap(), 0, ptr);
}
}
