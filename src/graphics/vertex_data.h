#pragma once

#include <array>
#include <cstdint>
#include <ranges>
#include <utility>
#include <vector>

#include "maths/vector3.h"

namespace ufps
{

struct UV
{
    float s;
    float t;
};

struct Weight
{
    std::uint32_t bone_index = 0zu;
    float weight = 0.0f;
};

struct VertexData
{
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    UV uv;
    std::array<Weight, 4zu> weights;
};

static_assert(
    sizeof(VertexData) == sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 3 + sizeof(float) * 3 +
                              sizeof(float) * 2 + sizeof(Weight) * 4);

template <class... Args>
constexpr auto vertices(Args &&...args) -> std::vector<ufps::VertexData>
{
    return std::views::zip_transform(
               []<class... A>(A &&...a) { return ufps::VertexData{std::forward<A>(a)...}; },
               std::forward<Args>(args)...) |
           std::ranges::to<std::vector>();
}

}
