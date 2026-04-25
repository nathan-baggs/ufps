#pragma once

#include <coroutine>
#include <stdexcept>
#include <type_traits>

#include "concurrency/awaitable_manager.h"
#include "utils/log.h"

namespace ufps
{

struct Task
{
    struct promise_type
    {
        auto initial_suspend() -> std::suspend_never
        {
            return {};
        }

        auto final_suspend() noexcept -> std::suspend_never
        {
            return {};
        }

        auto return_void() -> void
        {
        }

        auto unhandled_exception() -> void
        {
            ufps::log::error("unhandled exception in Task");
            AwaitableManager::last_exception() = std::current_exception();
        }

        auto get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    std::coroutine_handle<promise_type> handle;
};

}
