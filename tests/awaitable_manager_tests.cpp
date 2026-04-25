#include <atomic>
#include <chrono>

#include <gtest/gtest.h>

#include "concurrency/awaitable_manager.h"
#include "concurrency/task.h"
#include "concurrency/thread_pool.h"

using namespace std::literals;

TEST(awaitable_manager, await_next_tick)
{
    auto pool = ufps::ThreadPool{4u};
    auto am = ufps::AwaitableManager(pool);

    auto x = 0;

    const auto coro = [](ufps::AwaitableManager &am, int &x) -> ufps::Task
    {
        co_await am;
        x = 1;
    }(am, x);

    ASSERT_EQ(x, 0);

    am.pump();
    pool.drain();

    ASSERT_EQ(x, 1);
}

TEST(awaitable_manager, await_timer)
{
    auto pool = ufps::ThreadPool{4u};
    auto am = ufps::AwaitableManager(pool);

    auto x = std::atomic<int>{0};

    const auto coro = [](ufps::AwaitableManager &am, std::atomic<int> &x) -> ufps::Task
    {
        co_await am(500ms);
        x = 1;
    }(am, x);

    ASSERT_EQ(x, 0);

    const auto start = std::chrono::steady_clock::now();

    while (x == 0)
    {
        am.pump();
        pool.drain();
    }

    ASSERT_GE(std::chrono::steady_clock::now() - start, 500ms);
}

TEST(awaitable_manager, next_tick_exception_captured)
{
    auto pool = ufps::ThreadPool{4u};
    auto am = ufps::AwaitableManager(pool);

    const auto coro = [](ufps::AwaitableManager &am) -> ufps::Task
    {
        co_await am;
        throw 1;
    }(am);

    am.pump();
    pool.drain();

    ASSERT_THROW(am.pump(), int);
}

TEST(awaitable_manager, timer_exception_captured)
{
    auto pool = ufps::ThreadPool{4u};
    auto am = ufps::AwaitableManager(pool);

    const auto coro = [](ufps::AwaitableManager &am) -> ufps::Task
    {
        co_await am(500ms);
        throw 1;
    }(am);

    auto caught_exception = false;
    const auto start = std::chrono::steady_clock::now();

    while (!caught_exception)
    {
        try
        {
            am.pump();
        }
        catch (int)
        {
            caught_exception = true;
        }

        pool.drain();
    }

    ASSERT_TRUE(caught_exception);
    ASSERT_GE(std::chrono::steady_clock::now() - start, 500ms);
}
