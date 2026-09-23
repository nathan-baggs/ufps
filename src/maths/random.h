#pragma once

#include <cmath>
#include <concepts>
#include <contracts>
#include <numbers>
#include <random>
#include <ranges>

#include "maths/matrix3.h"
#include "maths/vector3.h"

namespace ufps
{

class Random
{
  public:
    Random(std::uint32_t seed = std::random_device{}())
        : generator_{seed}
    {
    }

    template <std::integral T>
    auto rand_int(T min, T max) -> T
    {
        auto dist = std::uniform_int_distribution<T>{min, max};
        return dist(generator_);
    }

    template <std::floating_point T>
    auto rand_real(T min, T max) -> T
    {
        auto dist = std::uniform_real_distribution<T>{min, max};
        return dist(generator_);
    }

    template <std::ranges::range T>
    const auto &rand_element(T &&elements)
    {
        contract_assert(!std::ranges::empty(elements));

        const auto index = rand_int(0zu, std::ranges::size(elements) - 1zu);
        return *(std::ranges::cbegin(elements) + index);
    }

    auto rand_vector3(const Vector3 &normal, float cone_half_angle) -> Vector3 //
        pre(normal.is_normalised() && !normal.is_zero())
    {
        const auto theta = rand_real(0.0f, std::numbers::pi_v<float> * 2.0f);
        const auto z = rand_real(std::cos(cone_half_angle), 1.0f);

        const auto local =
            Vector3{std::sqrt(1.0f - (z * z)) * std::cos(theta), std::sqrt(1.0f - (z * z)) * std::sin(theta), z};

        const auto reference = std::abs(normal.y) < 0.999f ? Vector3{0.0f, 1.0f, 0.0f} : Vector3{1.0f, 0.0f, 0.0f};

        const auto tangent = Vector3::normalise(Vector3::cross(normal, reference));
        const auto bitangent = Vector3::cross(normal, tangent);

        const auto rotate_mat = Matrix3{tangent, bitangent, normal};
        return Vector3::normalise(rotate_mat * local);
    }

  private:
    std::mt19937 generator_;
};

namespace random
{

inline auto instance() -> Random &
{
    thread_local Random r{};
    return r;
}

template <std::integral T>
auto rand_int(T min, T max) -> T
{
    return instance().rand_int(min, max);
}

template <std::floating_point T>
auto rand_real(T min, T max) -> T
{
    return instance().rand_real(min, max);
}

template <std::ranges::range T>
const auto &rand_element(T &&elements)
{
    return instance().rand_element(elements);
}

inline auto rand_vector3(const Vector3 &normal, float cone_half_angle) -> Vector3
{
    return instance().rand_vector3(normal, cone_half_angle);
}

}

}
