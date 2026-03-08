#pragma once

#include <span>
#include <string>
#include <vector>

#include "core/sub_mesh.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace ufps
{

class Entity
{
  public:
    Entity(std::string name, std::vector<SubMesh> sub_meshes, Transform transform);

    auto name() const -> std::string;
    auto sub_meshes() const -> std::span<const SubMesh>;
    auto transform() const -> const Transform &;
    auto set_transform(const Transform &transform) -> void;
    auto aabb() const -> const AABB &;

  private:
    std::string name_;
    std::vector<SubMesh> sub_meshes_;
    Transform transform_;
    AABB aabb_;
};

}
