#include "concurrency/thread_pool.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <processthreadsapi.h>
#include <stop_token>
#include <thread>

#include "utils/formatter.h"
#include "utils/log.h"

using namespace std::literals;

namespace ufps
{
ThreadPool::ThreadPool()
    : ThreadPool(std::clamp(std::jthread::hardware_concurrency() - 1u, 1u, 32u))
{
}

ThreadPool::ThreadPool(std::uint32_t worker_count)
    : worker_count_{worker_count}
    , job_queue_{}
    , worker_lock_{}
    , worker_cv_{}
    , job_count_{}
    , workers_{}
    , main_thread_{"main_thread", ::GetCurrentThread()}
    , profiler_thread_{
          {"profiler_thread", [this](std::stop_token stop_token) { profile_worker(std::move(stop_token)); }}}
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
    log::info("stopping threads");

    profiler_thread_->request_stop();
    profiler_thread_.reset();

    for (auto &thread : workers_)
    {
        thread.request_stop();
    }
}

auto ThreadPool::add(Job job) -> void
{
    ++job_count_;
    job_queue_.push(std::move(job));
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
        if (--job_count_ == 0u)
        {
            job_count_.notify_all();
        }
    }

    log::info("ending worker thread: {}", std::this_thread::get_id());
}

auto ThreadPool::profile_worker(std::stop_token stop_token) -> void
{
    log::info("starting profiler thread");

    std::this_thread::sleep_for(500ms);

    while (!stop_token.stop_requested())
    {
        for (auto &worker : workers_)
        {
            worker.stack_trace();
        }

        main_thread_.stack_trace();

        std::this_thread::sleep_for(1ms);
    }

    log::info("ending profiler thread");
}

auto ThreadPool::drain() const -> void
{
    auto count = job_count_.load();
    while (count != 0u)
    {
        job_count_.wait(count);
        count = job_count_.load();
    }
}

}
