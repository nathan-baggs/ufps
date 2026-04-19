#pragma once

#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

#include "core/camera.h"
#include "core/entity.h"
#include "graphics/material_manager.h"
#include "graphics/mesh_manager.h"
#include "graphics/point_light.h"
#include "graphics/texture_manager.h"
#include "maths/ray.h"
#include "maths/utils.h"
#include "maths/vector4.h"
#include "utils/string_map.h"

namespace ufps
{

struct IntersectionResult
{
    Entity *entity;
    Vector3 position;
    float distance;
};

struct LightData
{
    Colour ambient;
    std::vector<PointLight> lights;
};

struct ToneMapOptions
{
    float max_brightness;
    float contrast;
    float linear_section_start;
    float linear_section_length;
    float black_tightness;
    float pedestal;
    float gamma;
};

struct SSAOOptions
{
    bool enabled = true;
    std::uint32_t sample_count = 64u;
    float radius = 0.75f;
    float bias = 0.025f;
    float power = 2.0f;
};

struct ExposureOptions
{
    float min_log_luminance = -3.0f;
    float max_log_luminance = 1.0f;
};

class Scene
{
  public:
    struct Description
    {
        ToneMapOptions tone_map_options;
        SSAOOptions ssao_options;
        ExposureOptions exposure_options;
        LightData lights;
        std::vector<Entity::Description> entities;
    };

    Scene(
        MeshManager &mesh_manager,
        MaterialManager &material_manager,
        TextureManager &texture_manager,
        Camera camera,
        LightData lights,
        ToneMapOptions tone_map_options,
        SSAOOptions ssao_options,
        ExposureOptions exposure_options,
        const StringMap<Entity> &entity_cache);

    Scene(
        MeshManager &mesh_manager,
        MaterialManager &material_manager,
        TextureManager &texture_manager,
        Camera camera,
        const Description &description,
        const StringMap<Entity> &entity_cache);

    constexpr auto intersect_ray(const Ray &ray) -> std::optional<IntersectionResult>;

    auto create_entity(std::string_view name) -> Entity *;

    template <class Self>
    auto entities(this Self &&self)
    {
        using SpanType = std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Entity, Entity>;
        return std::span<SpanType>{self.entities_.data(), self.entities_.data() + self.entities_.size()};
    }

    auto cache_entity(std::string_view, Entity entity) -> void;

    constexpr auto &camera(this auto &&self)
    {
        return self.camera_;
    }

    constexpr auto &lights(this auto &&self)
    {
        return self.lights_;
    }

    constexpr auto &mesh_manager(this auto &&self)
    {
        return self.mesh_manager_;
    }

    constexpr auto &material_manager(this auto &&self)
    {
        return self.material_manager_;
    }

    constexpr auto &texture_manager(this auto &&self)
    {
        return self.texture_manager_;
    }

    constexpr auto add(PointLight light) -> void
    {
        lights_.lights.push_back(std::move(light));
    }

    constexpr auto &tone_map_options(this auto &&self)
    {
        return self.tone_map_options_;
    }

    constexpr auto &ssao_options(this auto &&self)
    {
        return self.ssao_options_;
    }

    constexpr auto &exposure_options(this auto &&self)
    {
        return self.exposure_options_;
    }

    constexpr auto description(this auto &&self) -> Description
    {
        return {
            .tone_map_options = self.tone_map_options_,
            .ssao_options = self.ssao_options_,
            .exposure_options = self.exposure_options_,
            .lights = self.lights_,
            .entities = self.entities_ | std::views::transform([](const auto &e) { return e.description(); }) |
                        std::ranges::to<std::vector>()};
    }

    constexpr auto remove(const Entity &entity) -> void
    {
        const auto iter = std::ranges::find_if(entities_, [&entity](const auto &e) { return &e == &entity; });
        expect(iter != std::ranges::cend(entities_), "entity not found");

        entities_.erase(iter);
    }

  private:
    std::vector<Entity> entities_;
    std::vector<Entity> entity_cache_;
    MeshManager &mesh_manager_;
    MaterialManager &material_manager_;
    TextureManager &texture_manager_;
    Camera camera_;
    LightData lights_;
    ToneMapOptions tone_map_options_;
    SSAOOptions ssao_options_;
    ExposureOptions exposure_options_;
};

constexpr auto Scene::intersect_ray(const Ray &ray) -> std::optional<IntersectionResult>
{
    auto result = std::optional<IntersectionResult>{};
    auto min_distance = std::numeric_limits<float>::max();

    for (auto &entity : entities_)
    {
        const auto inv_transform = Matrix4::invert(entity.transform());
        const auto transformed_ray =
            Ray{inv_transform * Vector4{ray.origin, 1.0f}, inv_transform * Vector4{ray.direction, 0.0f}};

        if (!!intersect(transformed_ray, entity.aabb()))
        {
            for (const auto &render_entity : entity.render_entities())
            {
                if (!intersect(transformed_ray, render_entity.aabb()))
                {
                    continue;
                }

                const auto mesh_view = render_entity.mesh_view();
                const auto indices = mesh_manager_.index_data(mesh_view);
                const auto vertices = mesh_manager_.vertex_data(mesh_view);

                for (const auto &indices : std::views::chunk(indices, 3))
                {
                    const auto v0 = vertices[indices[0]].position;
                    const auto v1 = vertices[indices[1]].position;
                    const auto v2 = vertices[indices[2]].position;

                    if (const auto distance = intersect(transformed_ray, v0, v1, v2); distance)
                    {
                        const auto intersection_point =
                            transformed_ray.origin + transformed_ray.direction * (*distance);

                        if (*distance < min_distance)
                        {
                            result = IntersectionResult{
                                .entity = &entity, .position = intersection_point, .distance = *distance};
                            min_distance = *distance;
                        }
                    }
                }
            }
        }
    }

    return result;
}

}
