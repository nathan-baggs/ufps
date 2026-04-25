#include <stack>

#include <gtest/gtest.h>

#include "concurrency/concurrent_queue.h"
#include "concurrency/thread.h"

TEST(concurrent_queue, ctor)
{
    auto q = ufps::ConcurrentQueue<int>{};

    ASSERT_TRUE(q.empty());
    ASSERT_EQ(q.size(), 0u);
}

TEST(concurrent_queue, push_front)
{
    auto q = ufps::ConcurrentQueue<int>{};

    q.push(1);
    q.push(2);
    q.push(3);

    ASSERT_FALSE(q.empty());
    ASSERT_EQ(q.size(), 3u);

    ASSERT_EQ(q.front(), 1);
    ASSERT_EQ(q.front(), 2);
    ASSERT_EQ(q.front(), 3);

    ASSERT_TRUE(q.empty());
}

TEST(concurrent_queue, push_top)
{
    auto q = ufps::ConcurrentQueue<int, std::stack<int>>{};

    q.push(1);
    q.push(2);
    q.push(3);

    ASSERT_FALSE(q.empty());
    ASSERT_EQ(q.size(), 3u);

    ASSERT_EQ(q.front(), 3);
    ASSERT_EQ(q.front(), 2);
    ASSERT_EQ(q.front(), 1);

    ASSERT_TRUE(q.empty());
}

TEST(concurrent_queue, concurrent_push_front)
{
    auto q = ufps::ConcurrentQueue<int>{};

    const auto push_func = [&q](std::stop_token, int start)
    {
        for (auto i = 0; i < 1000; ++i)
        {
            q.push(start + i);
        }
    };

    {
        auto thrd1 = ufps::Thread{"push_thread_1", push_func, 0};
        auto thrd2 = ufps::Thread{"push_thread_2", push_func, 1000};
    }

    ASSERT_FALSE(q.empty());
    ASSERT_EQ(q.size(), 2000u);
}

TEST(concurrent_queue, yield)
{
    auto q = ufps::ConcurrentQueue<int>{};

    q.push(1);
    q.push(2);
    q.push(3);

    auto yielded_q = q.yield();

    ASSERT_TRUE(q.empty());
    ASSERT_EQ(q.size(), 0u);

    ASSERT_FALSE(yielded_q.empty());
    ASSERT_EQ(yielded_q.size(), 3u);

    ASSERT_EQ(yielded_q.front(), 1);
    yielded_q.pop();
    ASSERT_EQ(yielded_q.front(), 2);
    yielded_q.pop();
    ASSERT_EQ(yielded_q.front(), 3);
    yielded_q.pop();

    ASSERT_TRUE(yielded_q.empty());
}
