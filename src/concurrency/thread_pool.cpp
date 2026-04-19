#include "concurrency/thread_pool.h"

#include <algorithm>
#include <format>
#include <stop_token>
#include <thread>

#include "utils/log.h"

namespace ufps
{
ThreadPool::ThreadPool()
    : ThreadPool(std::clamp(std::jthread::hardware_concurrency() - 1u, 1u, 32u))
{
}

ThreadPool::ThreadPool(std::uint32_t worker_count)
    : worker_count_{worker_count}
    , workers_{}
    , job_queue_{}
    , worker_lock_{}
    , worker_cv_{}
    , job_count_{}
    , jobs_complete_{true}
{
    log::info("starting thread pool with {} workers", worker_count);

    for (auto i = 0u; i < worker_count; ++i)
    {
        const auto name = std::format("worker_{}", i);
        workers_.push_back({name, [this](std::stop_token stop_token) { worker(std::move(stop_token)); }});
    }
}

ThreadPool::~ThreadPool()
{
    for (auto &thread : workers_)
    {
        thread.request_stop();
    }
}

auto ThreadPool::add(Job job) -> void
{
    job_queue_.push(std::move(job));
    ++job_count_;
    jobs_complete_ = false;
    worker_cv_.notify_one();
}

auto ThreadPool::worker_count() const -> std::uint32_t
{
    return worker_count_;
}

auto ThreadPool::worker(std::stop_token stop_token) -> void
{
    log::info("starting worker thread: {}", std::this_thread::get_id());

    while (!stop_token.stop_requested())
    {
        auto job = Job{};

        {
            auto lock = std::unique_lock(worker_lock_);
            worker_cv_.wait(lock, stop_token, [&] { return !job_queue_.empty(); });

            if (stop_token.stop_requested())
            {
                break;
            }

            job = job_queue_.front();
        }

        job();
        --job_count_;

        if (job_count_ == 0u)
        {
            jobs_complete_ = true;
            jobs_complete_.notify_one();
        }
    }

    log::info("ending worker thread: {}", std::this_thread::get_id());
}

auto ThreadPool::drain() const -> void
{
    while (job_count_ != 0u)
    {
        jobs_complete_.wait(false);
    }
}

}
