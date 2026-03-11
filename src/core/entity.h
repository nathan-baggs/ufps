#pragma once

#include <span>
#include <string>
#include <vector>

#include "core/render_entity.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace ufps
{

class Entity
{
  public:
    Entity(std::string name, std::vector<RenderEntity> render_entities, Transform transform);

    auto name() const -> std::string;
    auto render_entities() const -> std::span<const RenderEntity>;
    auto transform() const -> const Transform &;
    auto set_transform(const Transform &transform) -> void;
    auto aabb() const -> const AABB &;

  private:
    std::string name_;
    std::vector<RenderEntity> render_entities_;
    Transform transform_;
    AABB aabb_;
};

}
