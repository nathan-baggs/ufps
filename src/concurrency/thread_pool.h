#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

#include "concurrency/concurrent_queue.h"
#include "concurrency/cond_var.h"
#include "concurrency/lock.h"
#include "concurrency/thread.h"

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

  private:
    auto worker(std::stop_token stop_token) -> void;

    std::uint32_t worker_count_;
    std::vector<Thread> workers_;
    ConcurrentQueue<Job> job_queue_;
    Lock<> worker_lock_;
    CondVar worker_cv_;
    std::atomic<std::uint32_t> job_count_;
    std::atomic<bool> jobs_complete_;
};
}
