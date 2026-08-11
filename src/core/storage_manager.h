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

    constexpr auto insert(std::string_view name, T object) -> handle_type;

    constexpr auto operator[](std::string_view name) -> handle_type;

    constexpr auto operator[](handle_type handle);

    constexpr auto remove(handle_type handle) -> void;

    constexpr auto data();

    constexpr auto handles();

  private:
    SparseSet<T> objects_;
    StringMap<handle_type> object_names_;
};

template <class T>
constexpr auto StorageManager<T>::insert(std::string_view name, T object) -> handle_type
{
    auto handle = objects_.emplace(std::move(object));
    object_names_[std::string{name}] = handle;
    return handle;
}

template <class T>
constexpr auto StorageManager<T>::operator[](std::string_view name) -> handle_type
{
    auto element = object_names_.find(name);
    contract_assert(element != std::ranges::cend(object_names_));

    return element->second;
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
