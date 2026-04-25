#include <gtest/gtest.h>

#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"

auto eager(int *x) -> ufps::EagerTask
{
    *x = 1;
    co_return;
}

auto lazy(int *x) -> ufps::LazyTask
{
    *x = 1;
    co_return;
}

TEST(task, eager)
{
    auto x = int{};
    eager(&x);

    ASSERT_EQ(x, 1);
}

TEST(task, lazy)
{
    auto x = int{};
    auto task = lazy(&x);

    ASSERT_EQ(x, 0);

    task.handle.resume();

    ASSERT_EQ(x, 1);
}

TEST(task, exception_captured)
{
    auto task = [] -> ufps::EagerTask
    {
        throw 1;
        co_return;
    }();

    ASSERT_NE(ufps::AwaitableManager::last_exception, nullptr);
}
