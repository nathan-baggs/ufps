#pragma once

#include <functional>
#include <utility>

namespace ufps
{

template <class T, T Invalid = {}>
class AutoRelease
{
  public:
    AutoRelease()
        : AutoRelease(Invalid, nullptr)
    {
    }

    AutoRelease(T obj, std::function<void(T)> deleter)
        : obj_(obj)
        , deleter_(deleter)
    {
    }

    ~AutoRelease()
    {
        if ((obj_ != Invalid) && deleter_)
        {
            deleter_(obj_);
        }
    }

    AutoRelease(const AutoRelease &) = delete;
    auto operator=(const AutoRelease &) -> AutoRelease & = delete;

    AutoRelease(AutoRelease &&other)
        : AutoRelease()
    {
        swap(other);
    }

    auto operator=(AutoRelease &&other) -> AutoRelease &
    {
        AutoRelease new_obj{std::move(other)};
        swap(new_obj);

        return *this;
    }

    auto swap(AutoRelease &other) noexcept -> void
    {
        std::ranges::swap(obj_, other.obj_);
        std::ranges::swap(deleter_, other.deleter_);
    }

    auto reset(T obj) -> void
    {
        if ((obj_ != Invalid) && deleter_)
        {
            deleter_(obj_);
        }

        obj_ = obj;
    }

    T get() const
    {
        return obj_;
    }

    operator T() const
    {
        return obj_;
    }

    explicit operator bool() const
    {
        return obj_ != Invalid;
    }

    T *operator&() noexcept
    {
        return std::addressof(obj_);
    }

  private:
    T obj_;
    std::function<void(T)> deleter_;
};

}
