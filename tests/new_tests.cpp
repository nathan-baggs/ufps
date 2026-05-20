#include <new>
#include <windows.h>

#include <gtest/gtest.h>

namespace ufps
{
auto allocation_size(void *ptr) -> std::size_t;
}

TEST(memory, new_size)
{
    auto *x = ::operator new(100);
    ASSERT_GE(ufps::allocation_size(x), 100);

    ::operator delete(x);
}

TEST(memory, new_array)
{
    auto *x = ::operator new[](100);
    ASSERT_GE(ufps::allocation_size(x), 100);

    ::operator delete[](x);
}

TEST(memory, new_aligned_win32_default_alignment)
{
    auto *x = ::operator new(4, std::align_val_t{4});
    ASSERT_GE(ufps::allocation_size(x), 4);

    ::operator delete(x, std::align_val_t{4});
}

TEST(memory, new_aligned_win32_larger_alignment)
{
    auto *x = ::operator new(4, std::align_val_t{32});
    auto *orig_ptr = *(reinterpret_cast<void **>(x) - 1);
    ASSERT_GE(ufps::allocation_size(orig_ptr), 4 + 32 + sizeof(void *));

    ::operator delete(x, std::align_val_t{32});
}

TEST(memory, new_array_aligned_win32_default_alignment)
{
    auto *x = ::operator new[](4, std::align_val_t{4});
    ASSERT_GE(ufps::allocation_size(x), 4);

    ::operator delete(x, std::align_val_t{4});
}

TEST(memory, new_array_aligned_win32_larger_alignment)
{
    auto *x = ::operator new[](4, std::align_val_t{32});
    auto *orig_ptr = *(reinterpret_cast<void **>(x) - 1);
    ASSERT_GE(ufps::allocation_size(orig_ptr), 4 + 32 + sizeof(void *));

    ::operator delete(x, std::align_val_t{32});
}
