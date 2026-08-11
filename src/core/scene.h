#pragma once

#include <deque>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

#include "core/entity.h"
#include "core/entity_manager.h"
#include "core/light_manager.h"
#include "core/render_entity.h"
#include "core/render_entity_manager.h"
#include "core/service_locator.h"
#include "core/sparse_set.h"
#include "graphics/colour.h"
#include "graphics/mesh_manager.h"
#include "graphics/point_light.h"
#include "maths/bounded_number.h"
#include "maths/ray.h"
#include "maths/transform.h"
#include "maths/utils.h"
#include "maths/vector4.h"
#include "utils/string_map.h"

namespace ufps
{

struct IntersectionResult
{
    EntityHandle entity;
    Vector3 position;
    float distance;
};

struct ToneMapOptions
{
    BoundedFloat<0.0f, 100.0f> max_brightness = 5.0f;
    BoundedFloat<0.0f, 50.0f> contrast = 1.0f;
    BoundedFloat<0.0f, 1.0f> linear_section_start = 0.3f;
    BoundedFloat<0.0f, 1.0f> linear_section_length = 0.5f;
    BoundedFloat<0.0f, 3.0f> black_tightness = 0.0f;
    BoundedFloat<0.0f, 1.0f> pedestal = 0.0f;
    BoundedFloat<0.0f, 5.0f> gamma = 2.2f;
};

struct SSAOOptions
{
    bool enabled = true;
    BoundedUint32<1u, 64u> sample_count = 64u;
    BoundedFloat<0.1f, 2.0f> radius = 0.75f;
    BoundedFloat<0.01f, 0.1f> bias = 0.025f;
    BoundedFloat<1.0f, 4.0f> power = 2.0f;
};

struct ExposureOptions
{
    BoundedFloat<-10.0f, 10.0f> min_log_luminance = -3.0f;
    BoundedFloat<-10.0f, 10.0f> max_log_luminance = 1.0f;
};

struct FogOptions
{
    Colour colour = colours::black;
    BoundedFloat<0.0f, 0.2f> density = 0.005f;
};

struct ChromaticAberrationOptions
{
    BoundedFloat<-0.1f, 0.1f> red_offset = 0.009f;
    BoundedFloat<-0.1f, 0.1f> green_offset = 0.006f;
    BoundedFloat<-0.1f, 0.1f> blue_offset = -0.006f;
    BoundedFloat<0.0f, 1.0f> strength = 0.5f;
};

struct VignetteOptions
{
    Colour colour = colours::black;
    BoundedFloat<0.0f, 1.0f> strength = 0.5f;
    BoundedFloat<0.0f, 1.0f> feather = 0.1f;
};

struct FilmGrainOptions
{
    BoundedFloat<0.0f, 1.0f> strength = 0.01f;
};

struct BloomOptions
{
    BoundedFloat<0.0f, 0.1f> filter_radius = 0.005f;
    BoundedFloat<0.0f, 1.0f> mix_amount = 0.04f;
    BoundedFloat<0.0f, 10.0f> threshold = 1.0f;
};

class Scene
{
  public:
    struct Description
    {
        ToneMapOptions tone_map_options;
        SSAOOptions ssao_options;
        ExposureOptions exposure_options;
        FogOptions fog_options;
        ChromaticAberrationOptions chromatic_aberration_options;
        VignetteOptions vignette_options;
        FilmGrainOptions film_grain_options;
        BloomOptions bloom_options;
        Colour ambient;
        std::vector<PointLight> lights;
        std::vector<Entity::Description> entities;
    };

    constexpr Scene(const Description &description);

    constexpr auto intersect_ray(const Ray &ray) -> std::optional<IntersectionResult>;

    constexpr auto add(EntityHandle handle) -> void;

    constexpr auto &entities(this auto &&self);

    constexpr auto &ambient_light(this auto &&self);

    constexpr auto &tone_map_options(this auto &&self);

    constexpr auto &ssao_options(this auto &&self);

    constexpr auto &exposure_options(this auto &&self);

    constexpr auto &fog_options(this auto &&self);

    constexpr auto &chromatic_aberration_options(this auto &&self);

    constexpr auto &vignette_options(this auto &&self);

    constexpr auto &film_grain_options(this auto &&self);

    constexpr auto &bloom_options(this auto &&self);

    constexpr auto description(this auto &&self) -> Description;

    constexpr auto remove(EntityHandle handle) -> void;

  private:
    std::vector<EntityHandle> entities_;
    Colour ambient_;
    ToneMapOptions tone_map_options_;
    SSAOOptions ssao_options_;
    ExposureOptions exposure_options_;
    FogOptions fog_options_;
    ChromaticAberrationOptions chromatic_aberration_options_;
    VignetteOptions vignette_options_;
    FilmGrainOptions film_grain_options_;
    BloomOptions bloom_options_;
};

constexpr Scene::Scene(const Description &description)
    : entities_{}
    , ambient_{description.ambient}
    , tone_map_options_{description.tone_map_options}
    , ssao_options_{description.ssao_options}
    , exposure_options_{description.exposure_options}
    , fog_options_{description.fog_options}
    , chromatic_aberration_options_{description.chromatic_aberration_options}
    , vignette_options_{description.vignette_options}
    , film_grain_options_{description.film_grain_options}
    , bloom_options_{description.bloom_options}
{
    auto &&[em, rem, ps, lm] = services<EntityManager, RenderEntityManager, PhysicsSystem, LightManager>();

    for (auto &&[index, light] : std::views::enumerate(description.lights))
    {
        lm.insert(std::format("light{}", index), light);
        log::debug("inserted light");
    }

    for (const auto &entity_description : description.entities)
    {
        const auto new_entity_handle = em.insert(
            entity_description.name,
            {entity_description.name, rem[entity_description.name], entity_description.transform});

        auto new_entity = em[new_entity_handle];
        new_entity->set_emissive_strength(entity_description.emissive_strength);

        for (const auto &rb_description : entity_description.rigid_bodies)
        {
            const auto rb = ps.create_rigid_body(rb_description);
            new_entity->add_rigid_body(rb);
        }

        add(new_entity_handle);
    }
}

constexpr auto Scene::intersect_ray(const Ray &ray) -> std::optional<IntersectionResult>
{
    auto &&[mesh_manager, rem, em] = services<MeshManager, RenderEntityManager, EntityManager>();

    auto result = std::optional<IntersectionResult>{};
    auto min_distance = std::numeric_limits<float>::max();

    for (auto handle : entities_)
    {
        auto entity = em[handle];
        if (!entity)
        {
            continue;
        }

        const auto inv_transform = Matrix4::invert(entity->transform());
        const auto transformed_ray =
            Ray{inv_transform * Vector4{ray.origin, 1.0f}, inv_transform * Vector4{ray.direction, 0.0f}};

        if (!!intersect(transformed_ray, entity->aabb()))
        {
            for (auto render_entity_handle : entity->render_entities())
            {
                if (auto render_entity = rem[render_entity_handle]; render_entity)
                {
                    if (!intersect(transformed_ray, render_entity->aabb()))
                    {
                        continue;
                    }

                    const auto mesh_view = render_entity->mesh_view();
                    const auto indices = mesh_manager.index_data(mesh_view);
                    const auto vertices = mesh_manager.vertex_data(mesh_view);

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
                                    .entity = handle, .position = intersection_point, .distance = *distance};
                                min_distance = *distance;
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

constexpr auto Scene::add(EntityHandle handle) -> void
{
    entities_.push_back(handle);
}

constexpr auto &Scene::entities(this auto &&self)
{
    return self.entities_;
}

constexpr auto &Scene::ambient_light(this auto &&self)
{
    return self.ambient_;
}

constexpr auto &Scene::tone_map_options(this auto &&self)
{
    return self.tone_map_options_;
}

constexpr auto &Scene::ssao_options(this auto &&self)
{
    return self.ssao_options_;
}

constexpr auto &Scene::exposure_options(this auto &&self)
{
    return self.exposure_options_;
}

constexpr auto &Scene::fog_options(this auto &&self)
{
    return self.fog_options_;
}

constexpr auto &Scene::chromatic_aberration_options(this auto &&self)
{
    return self.chromatic_aberration_options_;
}

constexpr auto &Scene::vignette_options(this auto &&self)
{
    return self.vignette_options_;
}

constexpr auto &Scene::film_grain_options(this auto &&self)
{
    return self.film_grain_options_;
}

constexpr auto &Scene::bloom_options(this auto &&self)
{
    return self.bloom_options_;
}

constexpr auto Scene::description(this auto &&self) -> Description
{
    const auto &[em, lm] = services<EntityManager, LightManager>();

    return {
        .tone_map_options = self.tone_map_options_,
        .ssao_options = self.ssao_options_,
        .exposure_options = self.exposure_options_,
        .fog_options = self.fog_options_,
        .chromatic_aberration_options = self.chromatic_aberration_options_,
        .vignette_options = self.vignette_options_,
        .film_grain_options = self.film_grain_options_,
        .bloom_options = self.bloom_options_,
        .ambient = self.lights_.ambient,
        .lights = self.lights_.lights | std::views::filter([&](auto &e) { return !!lm[e]; }) |
                  std::views::transform([&](auto e) { return *em[e]; }) | std::ranges::to<std::vector>(),
        .entities = self.entities_ | std::views::filter([&](auto &e) { return !!em[e]; }) |
                    std::views::transform([&](auto e) { return em[e]->description(); }) |
                    std::ranges::to<std::vector>()};
}

constexpr auto Scene::remove(EntityHandle handle) -> void
{
    const auto iter = std::ranges::find(entities_, handle);
    expect(iter != std::ranges::cend(entities_), "entity not found");

    entities_.erase(iter);
}

}
