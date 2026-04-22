#pragma once

#include <coroutine>

#include "concurrency/concurrent_queue.h"
#include "concurrency/thread_pool.h"

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

    auto pump() -> void
    {
        auto to_process = next_tick_queue_.yield();

        while (!to_process.empty())
        {
            auto handle = std::move(to_process.front());
            to_process.pop();

            pool_.add([handle = std::move(handle)] { handle.resume(); });
        }
    }

  private:
    ThreadPool &pool_;
    ConcurrentQueue<std::coroutine_handle<>> next_tick_queue_;
};

}
