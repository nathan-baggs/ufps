#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <inplace_vector>
#include <optional>
#include <vector>

#include "concurrency/concurrent_queue.h"
#include "concurrency/cond_var.h"
#include "concurrency/lock.h"
#include "concurrency/thread.h"
#include "utils/stack_trace_counter.h"

namespace ufps
{

using Job = std::move_only_function<void()>;

class ThreadPool
{
  public:
    ThreadPool();
    ThreadPool(std::uint32_t worker_count);
    ~ThreadPool();

    auto add(Job job) -> void;

    auto worker_count() const -> std::uint32_t;

    auto drain() const -> void;

    auto profile_data();

  private:
    auto worker(std::stop_token stop_token) -> void;

    auto profile_worker(std::stop_token stop_token) -> void;

    std::uint32_t worker_count_;
    ConcurrentQueue<Job> job_queue_;
    Lock<> worker_lock_;
    CondVar worker_cv_;
    std::atomic<std::uint32_t> job_count_;
    std::vector<Thread> workers_;
    Thread main_thread_;
    std::optional<Thread> profiler_thread_;
    Lock<> profile_lock_;
    std::vector<StackTraceCounter> profile_data_;
};

inline auto ThreadPool::profile_data()
{
    const auto lck = std::scoped_lock{profile_lock_};

    const auto data_copy = profile_data_;
    return data_copy;
}

}
