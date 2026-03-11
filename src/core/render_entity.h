#pragma once

#include <cstdint>

#include "graphics/mesh_manager.h"
#include "graphics/mesh_view.h"
#include "maths/aabb.h"

namespace ufps
{
class RenderEntity
{
  public:
    RenderEntity(MeshView mesh_view, std::uint32_t material_index, const MeshManager &mesh_manager);

    auto mesh_view() const -> MeshView;
    auto material_index() const -> std::uint32_t;
    auto aabb() const -> const AABB &;

  private:
    MeshView mesh_view_;
    std::uint32_t material_index_;
    AABB aabb_;
};
}
