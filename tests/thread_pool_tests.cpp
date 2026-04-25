#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ranges>
#include <thread>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "concurrency/thread_pool.h"

using namespace std::literals;

TEST(thread_pool, ctor)
{
    auto pool = ufps::ThreadPool{};

    ASSERT_GT(pool.worker_count(), 0u);
}

TEST(thread_pool, simple)
{
    auto thread_ids = std::vector<std::thread::id>(4u, std::this_thread::get_id());

    auto pool = ufps::ThreadPool{4u};
    ASSERT_EQ(pool.worker_count(), 4u);

    for (auto i = 0u; i < 4u; ++i)
    {
        pool.add(
            [i, &thread_ids]
            {
                std::this_thread::sleep_for(200ms);
                thread_ids[i] = std::this_thread::get_id();
            });
    }

    pool.drain();

    const auto thread_ids_set = thread_ids | std::ranges::to<std::unordered_set>();
    ASSERT_EQ(thread_ids.size(), thread_ids_set.size());
    ASSERT_FALSE(std::ranges::any_of(thread_ids_set, [](const auto &e) { return e == std::this_thread::get_id(); }));
}
