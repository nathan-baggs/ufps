#pragma once

#include <algorithm>
#include <span>
#include <string>
#include <vector>

#include "core/render_entity.h"
#include "core/utils.h"
#include "maths/aabb.h"
#include "maths/transform.h"

namespace ufps
{

class Entity
{
  public:
    struct Description
    {
        std::string name;
        Transform transform;
        AABB aabb;
    };

    constexpr Entity(std::string name, std::vector<RenderEntity> render_entities, Transform transform);

    constexpr auto name() const -> std::string;
    constexpr auto render_entities() const -> std::span<const RenderEntity>;
    constexpr auto transform() const -> const Transform &;
    constexpr auto set_transform(const Transform &transform) -> void;
    constexpr auto aabb() const -> const AABB &;
    constexpr auto description() const -> Description;

  private:
    std::string name_;
    std::vector<RenderEntity> render_entities_;
    Transform transform_;
    AABB aabb_;
};

constexpr Entity::Entity(std::string name, std::vector<RenderEntity> render_entities, Transform transform)
    : name_{std::move(name)}
    , render_entities_{std::move(render_entities)}
    , transform_{std::move(transform)}
    , aabb_{create_aabb(render_entities_)}
{
}

constexpr auto Entity::name() const -> std::string
{
    return name_;
}

constexpr auto Entity::render_entities() const -> std::span<const RenderEntity>
{
    return render_entities_;
}

constexpr auto Entity::transform() const -> const Transform &
{
    return transform_;
}

constexpr auto Entity::set_transform(const Transform &transform) -> void
{
    transform_ = transform;
}

constexpr auto Entity::aabb() const -> const AABB &
{
    return aabb_;
}

constexpr auto Entity::description() const -> Entity::Description
{
    return {
        .name = name_,
        .transform = transform_,
        .aabb = aabb_,
    };
}

}
