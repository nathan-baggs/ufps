#pragma once

#include <exception>
#include <format>
#include <functional>
#include <inplace_vector>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>

#include <processthreadsapi.h>
#include <windows.h>
#include <winnt.h>

#include "utils/auto_release.h"
#include "utils/error.h"
#include "utils/exception.h"
#include "utils/log.h"
#include "utils/text_utils.h"

namespace ufps
{

class Thread
{
  public:
    Thread(std::string_view name, ::HANDLE handle)
        : name_{name}
        , exception_{}
        , stop_source_{}
        , handle_{handle, [](auto handle) { ::CloseHandle(handle); }}
    {
        const auto wide_name = text_widen(name_);
        ::SetThreadDescription(handle, wide_name.c_str());
    }

    template <class F, class... Args>
    Thread(std::string_view name, F &&func, Args &&...args)
        : name_{name}
        , exception_{}
        , stop_source_{}
        , handle_{
              nullptr,
              [](auto handle)
              {
                  ::WaitForSingleObject(handle, INFINITE);
                  ::CloseHandle(handle);
              }}
    {
        auto *thread_data = new std::tuple{
            auto(std::forward<F>(func)),
            auto(stop_source_.get_token()),
            std::addressof(exception_),
            auto(std::forward<Args>(args))...};
        using TupleType = std::remove_pointer_t<decltype(thread_data)>;

        auto thread_data_ptr = std::unique_ptr<TupleType>(thread_data);

        const auto new_thread = ::CreateThread(nullptr, 0zu, trampoline<TupleType>, thread_data_ptr.get(), 0, nullptr);
        if (new_thread == NULL)
        {
            throw Exception("failed to create thread: {}", name);
        }

        handle_.reset(new_thread);
        thread_data_ptr.release();

        const auto wide_name = text_widen(name_);

        ::SetThreadDescription(new_thread, wide_name.c_str());
    }

    ~Thread()
    {
        if (handle_ && stop_source_.stop_possible())
        {
            stop_source_.request_stop();
        }
    }

    Thread(const Thread &) = delete;
    auto operator=(const Thread &) -> Thread & = delete;
    Thread(Thread &&) = default;
    auto operator=(Thread &&) -> Thread & = default;

    auto request_stop() -> void
    {
        stop_source_.request_stop();
    }

    auto name() const -> std::string_view
    {
        return name_;
    }

    auto id() const
    {
        return 0;
    }

    auto to_string() const -> std::string
    {
        return std::format("thread: {} [{}]", name_, id());
    }

    auto stack_trace() -> std::inplace_vector<void *, 100>
    {
        auto callstack = std::inplace_vector<void *, 100>{};

        auto thread_handle = AutoRelease<::HANDLE, nullptr>{nullptr, [](auto h) { ::CloseHandle(h); }};

        ::SuspendThread(handle_);

        // BEGIN DANGER ZONE - https://www.youtube.com/watch?v=siwpn14IE7E

        const auto success = [&, this]
        {
            thread_handle.reset(::OpenThread(THREAD_GET_CONTEXT, false, ::GetThreadId(handle_)));
            if (!thread_handle)
            {
                return false;
            }

            auto context = ::CONTEXT{};
            context.ContextFlags = CONTEXT_FULL;

            if (::GetThreadContext(thread_handle, &context) == 0)
            {
                return false;
            }

            while (context.Rip != 0 && context.Rsp != 0)
            {
                if (callstack.try_push_back(reinterpret_cast<void *>(context.Rip)) == nullptr)
                {
                    return false;
                }

                auto table = ::UNWIND_HISTORY_TABLE{};
                auto image_base = ::DWORD64{};
                auto *func = ::RtlLookupFunctionEntry(context.Rip, &image_base, &table);

                if (func == nullptr)
                {
                    const auto *rip_ptr = reinterpret_cast<const ::DWORD64 *>(context.Rsp);
                    context.Rip = *rip_ptr;
                    context.Rsp += sizeof(context.Rsp);
                    continue;
                }
                else
                {
                    void *handler_data{};
                    auto establisher_frame = ::DWORD64{};

                    ::RtlVirtualUnwind(
                        UNW_FLAG_NHANDLER,
                        image_base,
                        context.Rip,
                        func,
                        &context,
                        &handler_data,
                        &establisher_frame,
                        nullptr);
                }
            }

            return true;
        }();

        // END DANGER ZONE

        ::ResumeThread(handle_);

        ensure(success, "failed to get stack trace");

        return callstack;
    }

    auto exception() const -> std::exception_ptr
    {
        return exception_;
    }

  private:
    template <class T>
    static auto trampoline(void *data) -> ::DWORD
    {
        auto *thread_data = static_cast<T *>(data);
        auto thread_data_ptr = std::unique_ptr<T>(thread_data);

        auto ret_value = ::DWORD{};

        std::apply(
            [&ret_value]<class F, class... Args>(
                F &&f, std::stop_token stop_token, std::exception_ptr *exception, Args &&...args)
            {
                try
                {
                    std::invoke(std::forward<F>(f), stop_token, std::forward<Args>(args)...);
                }
                catch (...)
                {
                    *exception = std::current_exception();
                    ret_value = 1;
                }
            },
            *thread_data_ptr);

        return ret_value;
    }

    std::string name_;
    std::exception_ptr exception_;
    std::stop_source stop_source_;
    AutoRelease<::HANDLE, nullptr> handle_;
};

}
