#pragma once

#include <atomic>
#include <concepts>
#include <cstdint>
#include <queue>

#include "concurrency/lock.h"

namespace ufps
{

namespace impl
{
template <class T>
concept CanFront = requires(T t) { t.front(); };

template <class T>
concept CanTop = requires(T t) { t.top(); };

}

template <class T, class Q = std::queue<T>>
class ConcurrentQueue
{
  public:
    auto front() -> T
        requires impl::CanFront<Q>
    {
        const auto lock = std::scoped_lock{lock_};
        --size_;
        auto obj = std::move(q_.front());
        q_.pop();

        return obj;
    }

    auto front() -> T
        requires impl::CanTop<Q>
    {
        const auto lock = std::scoped_lock{lock_};
        --size_;
        auto obj = std::move(q_.top());
        q_.pop();

        return obj;
    }

    auto push(T &&obj) -> void
    {
        const auto lock = std::scoped_lock{lock_};
        ++size_;
        q_.push(std::forward<T>(obj));
    }

    auto empty() const -> bool
    {
        return size_ == 0u;
    }

    auto size() const -> std::uint32_t
    {
        return size_;
    }

    auto yield() -> Q
    {
        auto q = Q{};

        {
            const auto lock = std::scoped_lock{lock_};
            std::ranges::swap(q, q_);
            size_ = 0u;
        }

        return q;
    }

  private:
    Q q_;
    Lock<> lock_;
    std::atomic<std::uint32_t> size_;
};

}
