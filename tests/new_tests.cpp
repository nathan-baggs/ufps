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
