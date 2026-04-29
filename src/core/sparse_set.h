#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

#include "utils/error.h"

namespace ufps
{

template <class T, class Allocator = std::allocator<T>>
class SparseSet
{
    class Handle
    {
        inline static constexpr auto Invalid = std::numeric_limits<std::uint32_t>::max();

        constexpr Handle()
            : Handle(Invalid)
        {
        }

        explicit constexpr Handle(std::uint32_t index, std::uint32_t version)
            : index_{index}
            , version_{version}
        {
        }

        constexpr auto operator<=>(const Handle &) const = default;

        std::uint32_t index_;
        std::uint32_t version_;

        friend SparseSet;
    };

  public:
    using value_type = T;
    using handle_type = Handle;

    constexpr SparseSet();

    template <class... Args>
    constexpr auto emplace(Args &&...args) -> handle_type
    {
        const auto dense_index = static_cast<std::uint32_t>(std::ranges::size(data_));
        data_.emplace_back(std::forward<Args>(args)...);

        auto sparse_index = static_cast<std::uint32_t>(std::ranges::size(sparse_));
        auto version = 0u;

        if (!std::ranges::empty(free_))
        {
            sparse_index = free_.back();
            sparse_[sparse_index].index_ = dense_index;
            ++sparse_[sparse_index].version_;
            version = sparse_[sparse_index].version_;
            free_.pop_back();
        }
        else
        {
            sparse_.push_back(handle_type{dense_index, version});
        }

        dense_.push_back(sparse_index);

        return handle_type{sparse_index, version};
    }

    template <class S>
    constexpr auto operator[](this S &&self, handle_type handle)
    {
        using RetType =
            std::conditional_t<std::is_const_v<std::remove_reference_t<S>>, const value_type &, value_type &>;

        if (std::ranges::empty(self.dense_))
        {
            return std::optional<RetType>{};
        }

        const auto sparse_index = handle.index_;
        if (sparse_index >= std::ranges::size(self.sparse_))
        {
            return std::optional<RetType>{};
        }

        const auto dense_index = self.sparse_[sparse_index].index_;
        if (dense_index >= std::ranges::size(self.dense_))
        {
            return std::optional<RetType>{};
        }

        if (self.dense_[dense_index] != sparse_index || handle.version_ != self.sparse_[sparse_index].version_)
        {
            return std::optional<RetType>{};
        }

        return std::optional<RetType>(self.data_[dense_index]);
    }

    constexpr auto remove(handle_type handle);

    constexpr auto size() const -> std::size_t;

    constexpr auto empty() const -> bool;

    constexpr auto handles() const -> std::vector<handle_type>;

    constexpr auto data() const -> std::span<const T>;

  private:
    template <class U>
    using VectorRebind = std::vector<U, typename std::allocator_traits<Allocator>::template rebind_alloc<U>>;
    VectorRebind<handle_type> sparse_;
    VectorRebind<std::uint32_t> dense_;
    std::vector<T, Allocator> data_;
    VectorRebind<std::size_t> free_;
};

template <class T, class Allocator>
constexpr SparseSet<T, Allocator>::SparseSet()
    : sparse_{}
    , dense_{}
    , data_{}
{
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::size() const -> std::size_t
{
    return std::ranges::size(data_);
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::empty() const -> bool
{
    return std::ranges::empty(data_);
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::remove(handle_type handle)
{
    const auto sparse_index = handle.index_;
    ensure(sparse_index < std::ranges::size(sparse_), "invalid handle: {}", sparse_index);

    const auto dense_index = sparse_[sparse_index].index_;
    ensure(dense_index < std::ranges::size(dense_), "invalid handle: {}", sparse_index);

    ensure(dense_[dense_index] == sparse_index, "invalid handle: {}", sparse_index);

    if (dense_index == std::ranges::size(data_) - 1u)
    {
        data_.pop_back();
        dense_.pop_back();
        sparse_[sparse_index].index_ = handle_type::Invalid;
        free_.push_back(sparse_index);

        return;
    }

    std::ranges::swap(data_[sparse_[sparse_index].index_], *(std::ranges::end(data_) - 1u));
    data_.pop_back();
    std::ranges::swap(dense_[sparse_[sparse_index].index_], *(std::ranges::end(dense_) - 1u));
    dense_.pop_back();

    sparse_[sparse_index].index_ = handle_type::Invalid;
    free_.push_back(sparse_index);

    if (!std::ranges::empty(dense_))
    {
        sparse_[dense_[dense_index]].index_ = dense_index;
    }
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::handles() const -> std::vector<handle_type>
{
    // we use handle_type in two ways, to store an index into the dense array internally and as an index to the sparse
    // array which we return to the user
    // here we convert from the internal representation to the user-facing one by replacing the index with the correct
    // one if it's valid and filtering out invalid handles
    return std::views::enumerate(sparse_) |
           std::views::transform(
               [](const auto &e)
               {
                   const auto &[index, handle] = e;
                   const auto correct_index = handle.index_ == handle_type::Invalid ? handle.index_ : index;
                   return handle_type{static_cast<std::uint32_t>(correct_index), handle.version_};
               }) |
           std::views::filter([](const auto &e) { return e.index_ != handle_type::Invalid; }) |
           std::ranges::to<std::vector>();
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::data() const -> std::span<const T>
{
    return data_;
}

}
