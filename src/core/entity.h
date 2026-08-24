#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "core/camera_manager.h"
#include "core/light_manager.h"
#include "core/render_entity.h"
#include "core/render_entity_manager.h"
#include "core/service_locator.h"
#include "core/utils.h"
#include "maths/aabb.h"
#include "maths/transform.h"
#include "physics/physics_system.h"

namespace ufps
{

class Entity
{
    using EntityHandle = SparseSet<Entity>::handle_type;

  public:
    struct Description
    {
        std::string name;
        float emissive_strength;
        Transform transform;
        AABB aabb;
        std::vector<RigidBody::Description> rigid_bodies;
        std::vector<std::string> render_entities;
        std::vector<std::string> children;
        std::optional<Camera::Description> camera;
        std::optional<PointLight> light;
    };

    Entity(std::string name, std::span<const RenderEntityHandle> render_entities, Transform transform);

    auto name() const -> std::string_view;

    auto set_name(std::string name) -> void;

    auto render_entities() const -> std::span<const RenderEntityHandle>;

    auto add_render_entities(std::span<const RenderEntityHandle> render_entities) -> void;

    auto remove_render_entity(RenderEntityHandle handle) -> void //
        pre(std::ranges::contains(render_entities_, handle));

    auto transform() const -> const Transform &;

    auto local_transform() const -> const Transform &;

    auto parent_transform() const -> const Transform &;

    auto set_transform(const Transform &transform) -> void;

    auto aabb() const -> const AABB &;

    auto description() const -> Description;

    auto emissive_strength() const -> float;

    auto set_emissive_strength(float strength) -> void;

    auto add_rigid_body(RigidBodyHandle handle) -> void;

    auto rigid_bodies() const -> std::span<const RigidBodyHandle>;

    auto light() const -> LightHandle;

    auto set_light(LightHandle handle) -> void;

    auto add_child(EntityHandle child) -> void;

    auto children() -> std::span<const EntityHandle>;

    auto camera() const -> CameraHandle;

    auto set_camera(CameraHandle handle) -> void;

  private:
    auto update_transforms(const Transform &local, const Transform &parent) -> void;

    auto set_parent_transform(const Transform &transform) -> void;

    std::string name_;
    std::vector<RenderEntityHandle> render_entities_;
    std::vector<RigidBodyHandle> rigid_bodies_;
    LightHandle light_;
    Transform local_transform_;
    Transform parent_transform_;
    Transform transform_;
    AABB aabb_;
    float emissive_strength_;
    std::vector<EntityHandle> children_;
    CameraHandle camera_;
};

}
