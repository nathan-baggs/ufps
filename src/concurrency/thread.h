#pragma once

#include <errhandlingapi.h>
#include <exception>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include <processthreadsapi.h>
#include <windows.h>

namespace ufps
{

class Thread
{
  public:
    template <class F, class... Args>
    Thread(std::string_view name, F &&func, Args &&...args)
        : name_{name}
        , exception_{}
        , thread_{
              [this]<class... A>(std::stop_token stop_token, F &&func, A &&...a)
              {
                  try
                  {
                      func(stop_token, std::forward<A>(a)...);
                  }
                  catch (...)
                  {
                      exception_ = std::current_exception();
                  }
              },
              std::forward<F>(func),
              std::forward<Args>(args)...}
    {
    }

    auto request_stop() -> void
    {
        thread_.request_stop();
    }

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto id() const
    {
        return thread_.get_id();
    }

    auto to_string() const -> std::string
    {
        return std::format("thread: {} [{}]", name_, thread_.get_id());
    }

    auto exception() const -> std::exception_ptr
    {
        return exception_;
    }

  private:
    std::string name_;
    std::exception_ptr exception_;
    std::jthread thread_;
};

}
