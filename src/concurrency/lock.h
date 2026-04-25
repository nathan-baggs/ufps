#pragma once

#include <mutex>

namespace ufps
{

template <class M = std::mutex>
class Lock
{
  public:
    auto lock() -> void
    {
        lock_.lock();
    }

    auto unlock() -> void
    {
        lock_.unlock();
    }

  private:
    M lock_;
};

}
