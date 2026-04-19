#pragma once

#include <condition_variable>
#include <stop_token>

#include "concurrency/lock.h"

namespace ufps
{

class CondVar
{
  public:
    auto notify_one() -> void;
    auto notify_all() -> void;

    template <class T, class F>
    auto wait(T &lock, std::stop_token stop_token, F &&f) -> void
    {
        cv_.wait(lock, stop_token, std::forward<F>(f));
    }

  private:
    std::condition_variable_any cv_;
};

}
