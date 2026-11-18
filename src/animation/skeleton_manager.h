#pragma once

#include <algorithm>
#include <ranges>
#include <vector>

#include "animation/skeleton.h"
#include "core/sparse_set.h"
#include "core/storage_manager.h"
#include "utils/error.h"

namespace ufps
{

using SkeletonHandle = SparseSet<Skeleton>::handle_type;

class SkeletonManager : public StorageManager<Skeleton>
{
  public:
    using StorageManager<Skeleton>::operator[];

    constexpr auto operator[](std::string_view name) const -> SkeletonHandle;
};

constexpr auto SkeletonManager::operator[](std::string_view name) const -> SkeletonHandle
{
    const auto hndls = handles();
    const auto skeleton = std::ranges::find_if(
        hndls,
        [&](const auto &h)
        {
            if (const auto skl = StorageManager<Skeleton>::operator[](h); skl)
            {
                return skl->name() == name;
            }

            return false;
        });

    return skeleton == std::ranges::cend(hndls) ? SkeletonHandle{} : *skeleton;
}

}
