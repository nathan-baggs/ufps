#include <algorithm>
#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "concurrency/thread.h"
#include "utils/resolve_symbols.h"

using namespace std::literals;

TEST(thread, simple)
{
    auto thread_id = std::this_thread::get_id();

    {
        auto thrd =
            ufps::Thread{"test_thread", [&thread_id](std::stop_token) { thread_id = std::this_thread::get_id(); }};
        ASSERT_EQ(thrd.name(), "test_thread");
    }

    ASSERT_NE(thread_id, std::this_thread::get_id());
}

TEST(thread, stop)
{
    auto counter = 0u;

    {
        auto thrd = ufps::Thread{
            "test_thread",
            [&counter](std::stop_token stop_token)
            {
                while (!stop_token.stop_requested())
                {
                    ++counter;
                    std::this_thread::sleep_for(100ms);
                }
            }};

        std::this_thread::sleep_for(200ms);

        thrd.request_stop();
    }

    ASSERT_GT(counter, 0u);
}

TEST(thread, exception)
{
    auto counter = 0u;

    auto thrd = ufps::Thread{"test_thread", [&counter](std::stop_token stop_token) { throw 1; }};

    std::this_thread::sleep_for(200ms);

    ASSERT_NE(thrd.exception(), nullptr);
}

TEST(thread, callstack)
{
    auto ready = std::atomic<bool>{false};

    auto thrd = ufps::Thread{
        "test_thread",
        [&](std::stop_token stop_token)
        {
            [&]()
            {
                [&]()
                {
                    [&]()
                    {
                        ready = true;
                        ready.notify_one();
                        while (!stop_token.stop_requested())
                        {
                        }
                    }();
                }();
            }();
        }};

    ready.wait(false);

    auto stack_trace = thrd.stack_trace();
    ASSERT_GE(std::ranges::size(stack_trace), 4zu);

    const auto symbols = ufps::resolve_symbols(stack_trace);
    ASSERT_GE(std::ranges::size(stack_trace), 4zu);

    thrd.request_stop();
}
