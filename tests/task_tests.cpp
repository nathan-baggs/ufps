#include <gtest/gtest.h>

#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"

TEST(task, simple)
{
    auto x = int{};

    const auto coro = [](int &x) -> ufps::Task
    {
        x = 1;
        co_return;
    }(x);

    ASSERT_EQ(x, 1);
}

TEST(task, exception_captured)
{
    auto task = [] -> ufps::Task
    {
        throw 1;
        co_return;
    }();

    ASSERT_NE(ufps::AwaitableManager::last_exception, nullptr);
}
