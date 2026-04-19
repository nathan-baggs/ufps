#pragma once

#include <atomic>
#include <cstdint>
#include <queue>

#include "concurrency/lock.h"

namespace ufps
{

template <class T, class Q = std::queue<T>>
class ConcurrentQueue
{
  public:
    auto front() -> T
    {
        const auto lock = std::scoped_lock{lock_};
        --size_;
        auto obj = std::move(q_.front());
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

  private:
    Q q_;
    Lock<> lock_;
    std::atomic<std::uint32_t> size_;
};

}
