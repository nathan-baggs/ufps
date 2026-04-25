#pragma once

#include <chrono>
#include <coroutine>
#include <queue>

#include "concurrency/concurrent_queue.h"
#include "concurrency/thread_pool.h"
#include "utils/log.h"

namespace ufps
{

class AwaitableManager
{
  public:
    AwaitableManager(ThreadPool &pool)
        : pool_{pool}
        , next_tick_queue_{}
    {
    }

    ~AwaitableManager()
    {
        auto to_process = next_tick_queue_.yield();

        while (!to_process.empty())
        {
            to_process.front().destroy();
            to_process.pop();
        }
    }

    AwaitableManager(const AwaitableManager &) = delete;
    auto operator=(const AwaitableManager &) -> AwaitableManager & = delete;

    auto operator co_await()
    {
        struct Awaitable
        {
            auto await_ready()
            {
                return false;
            }

            auto await_suspend(std::coroutine_handle<> h)
            {
                self.next_tick_queue_.push(std::move(h));
            }

            auto await_resume()
            {
            }

            AwaitableManager &self;
        };

        return Awaitable{*this};
    }

    auto operator()(std::chrono::nanoseconds wait_time)
    {
        struct Awaitable
        {
            auto await_ready()
            {
                return false;
            }

            auto await_suspend(std::coroutine_handle<> h)
            {
                self.timer_queue_.push({
                    .time_point = std::chrono::steady_clock::now() + wait_time,
                    .handle = h,
                });
            }

            auto await_resume()
            {
            }

            AwaitableManager &self;
            std::chrono::nanoseconds wait_time;
        };

        return Awaitable{*this, wait_time};
    }

    auto pump() -> void
    {
        auto to_process = next_tick_queue_.yield();

        while (!to_process.empty())
        {
            auto handle = std::move(to_process.front());
            to_process.pop();

            pool_.add(
                [this, handle]
                {
                    handle.resume();
                    if (last_exception())
                    {
                        ufps::log::error("unhandled exception in next tick awaitable");
                        exception_queue_.push(std::exchange(last_exception(), nullptr));
                    }
                });
        }

        while (!timer_queue_.empty())
        {
            auto timer_awaitable = timer_queue_.front();
            if (timer_awaitable.time_point <= std::chrono::steady_clock::now())
            {
                pool_.add(
                    [this, handle = timer_awaitable.handle]
                    {
                        handle.resume();
                        if (last_exception())
                        {
                            ufps::log::error("unhandled exception in timer awaitable");
                            exception_queue_.push(std::exchange(last_exception(), nullptr));
                        }
                    });
            }
            else
            {
                timer_queue_.push(std::move(timer_awaitable));
                break;
            }
        }

        if (!exception_queue_.empty())
        {
            auto exception = exception_queue_.front();
            std::rethrow_exception(exception);
        }
    }

    static std::exception_ptr &last_exception()
    {
        thread_local auto instance = std::exception_ptr{};
        return instance;
    }

  private:
    struct TimerAwaitable
    {
        std::chrono::steady_clock::time_point time_point;
        std::coroutine_handle<> handle;

        auto operator>(const TimerAwaitable &other) const -> bool
        {
            return time_point > other.time_point;
        }
    };
    using TimePriorityQueue =
        std::priority_queue<TimerAwaitable, std::vector<TimerAwaitable>, std::greater<TimerAwaitable>>;

    ThreadPool &pool_;
    ConcurrentQueue<std::coroutine_handle<>> next_tick_queue_;
    ConcurrentQueue<TimerAwaitable, TimePriorityQueue> timer_queue_;
    ConcurrentQueue<std::exception_ptr> exception_queue_;
};

}
