#pragma once

#include <coroutine>
#include <type_traits>

namespace ufps
{

template <bool Eager>
struct Task
{
    struct promise_type
    {
        auto initial_suspend()
        {
            return std::conditional_t<Eager, std::suspend_never, std::suspend_always>{};
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
        }

        auto get_return_object()
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    std::coroutine_handle<promise_type> handle;
};

using EagerTask = Task<true>;
using LazyTask = Task<false>;

}
