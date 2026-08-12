#pragma once

#include "core/sparse_set.h"
#include "utils/error.h"
#include "utils/string_map.h"

namespace ufps
{

template <class T>
class StorageManager
{
  public:
    using handle_type = SparseSet<T>::handle_type;

    constexpr auto insert(T object) -> handle_type;

    constexpr auto operator[](handle_type handle);

    constexpr auto remove(handle_type handle) -> void;

    constexpr auto data();

    constexpr auto handles();

  private:
    SparseSet<T> objects_;
};

template <class T>
constexpr auto StorageManager<T>::insert(T object) -> handle_type
{
    auto handle = objects_.emplace(std::move(object));
    return handle;
}

template <class T>
constexpr auto StorageManager<T>::operator[](handle_type handle)
{
    return objects_[handle];
}

template <class T>
constexpr auto StorageManager<T>::remove(handle_type handle) -> void
{
    objects_.remove(handle);
}

template <class T>
constexpr auto StorageManager<T>::data()
{
    return objects_.data();
}

template <class T>
constexpr auto StorageManager<T>::handles()
{
    return objects_.handles();
}

}
