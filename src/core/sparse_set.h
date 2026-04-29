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
        inline static constexpr auto Invalid = std::numeric_limits<std::size_t>::max();

        constexpr Handle()
            : Handle(Invalid)
        {
        }

        explicit constexpr Handle(std::size_t index)
            : index_{index}
        {
        }

        constexpr auto operator<=>(const Handle &) const = default;

        std::size_t index_;

        friend SparseSet;
    };

  public:
    using value_type = T;
    using handle_type = Handle;

    constexpr SparseSet();

    template <class... Args>
    constexpr auto emplace(Args &&...args) -> handle_type
    {
        const auto dense_index = std::ranges::size(data_);
        data_.emplace_back(std::forward<Args>(args)...);

        const auto sparse_index = std::ranges::size(sparse_);
        sparse_.push_back(dense_index);

        dense_.push_back(sparse_index);

        return handle_type{sparse_index};
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

        const auto dense_index = self.sparse_[sparse_index];
        if (dense_index >= std::ranges::size(self.dense_))
        {
            return std::optional<RetType>{};
        }

        if (self.dense_[dense_index] != sparse_index)
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
    using VectorRebind = std::vector<U, typename std::allocator_traits<Allocator>::template rebind_alloc<std::size_t>>;
    VectorRebind<std::size_t> sparse_;
    VectorRebind<std::size_t> dense_;
    std::vector<T, Allocator> data_;
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

    const auto dense_index = sparse_[sparse_index];
    ensure(dense_index < std::ranges::size(dense_), "invalid handle: {}", sparse_index);

    ensure(dense_[dense_index] == sparse_index, "invalid handle: {}", sparse_index);

    if (dense_index == std::ranges::size(data_) - 1zu)
    {
        data_.pop_back();
        dense_.pop_back();
        sparse_[sparse_index] = handle_type::Invalid;

        return;
    }

    std::ranges::swap(data_[sparse_[sparse_index]], *(std::ranges::end(data_) - 1zu));
    data_.pop_back();
    std::ranges::swap(dense_[sparse_[sparse_index]], *(std::ranges::end(dense_) - 1zu));
    dense_.pop_back();

    sparse_[sparse_index] = handle_type::Invalid;

    if (!std::ranges::empty(dense_))
    {
        sparse_[std::ranges::size(sparse_) - 1zu] = dense_index;
    }
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::handles() const -> std::vector<handle_type>
{
    return sparse_ |                                                                     //
           std::views::filter([](const auto &e) { return e != handle_type::Invalid; }) | //
           std::views::transform([](const auto &e) { return handle_type{e}; }) |         //
           std::ranges::to<std::vector>();
}

template <class T, class Allocator>
constexpr auto SparseSet<T, Allocator>::data() const -> std::span<const T>
{
    return data_;
}

}
