#pragma once

#include <array>
#include <format>
#include <ranges>
#include <span>

#include "maths/quaternion.h"
#include "maths/vector3.h"
#include "utils/error.h"

namespace ufps
{

class Matrix4
{
  public:
    struct Scale
    {
    };

    constexpr Matrix4()
        : elements_({
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
          })
    {
    }

    explicit constexpr Matrix4(const std::array<float, 16u> &elements)
        : Matrix4{std::span<const float>{elements}}
    {
    }

    explicit constexpr Matrix4(const std::span<const float> &elements)
        : Matrix4{}
    {
        ensure(elements.size() == 16u, "not enough elements");
        std::ranges::copy(elements, std::ranges::begin(elements_));
    }

    explicit constexpr Matrix4(const Vector3 &translation)
        : elements_({
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
              0.0f,
              translation.x,
              translation.y,
              translation.z,
              1.0f,
          })
    {
    }

    constexpr Matrix4(const Vector3 &scale, Scale)
        : elements_({
              scale.x,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              scale.y,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              scale.z,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
          })
    {
    }

    constexpr Matrix4(const Quaternion &rotation)
        : Matrix4{}
    {
        elements_[0] = 1.0f - 2.0f * rotation.y * rotation.y - 2.0f * rotation.z * rotation.z;
        elements_[1] = 2.0f * rotation.x * rotation.y + 2.0f * rotation.z * rotation.w;
        elements_[2] = 2.0f * rotation.x * rotation.z - 2.0f * rotation.y * rotation.w;

        elements_[4] = 2.0f * rotation.x * rotation.y - 2.0f * rotation.z * rotation.w;
        elements_[5] = 1.0f - 2.0f * rotation.x * rotation.x - 2.0f * rotation.z * rotation.z;
        elements_[6] = 2.0f * rotation.y * rotation.z + 2.0f * rotation.x * rotation.w;

        elements_[8] = 2.0f * rotation.x * rotation.z + 2.0f * rotation.y * rotation.w;
        elements_[9] = 2.0f * rotation.y * rotation.z - 2.0f * rotation.x * rotation.w;
        elements_[10] = 1.0f - 2.0f * rotation.x * rotation.x - 2.0f * rotation.y * rotation.y;
    }

    constexpr Matrix4(const Vector3 &translation, const Vector3 &scale)
        : elements_({
              scale.x,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              scale.y,
              0.0f,
              0.0f,
              0.0f,
              0.0f,
              scale.z,
              0.0f,
              translation.x,
              translation.y,
              translation.z,
              1.0f,
          })
    {
    }

    static constexpr auto look_at(const Vector3 &eye, const Vector3 &look_at, const Vector3 &up) -> Matrix4;

    static constexpr auto perspective(float fov, float width, float height, float near_plane, float far_plane)
        -> Matrix4;
    static constexpr auto orthographic(float width, float height, float depth) -> Matrix4;

    constexpr auto data() const -> std::span<const float>
    {
        return elements_;
    }

    constexpr auto operator[](this auto &&self, std::size_t index) -> auto &
    {
        return self.elements_[index];
    }

    friend constexpr auto operator*=(Matrix4 &m1, const Matrix4 &m2) -> Matrix4 &;

    constexpr auto operator==(const Matrix4 &) const -> bool = default;

    auto to_string() const -> std::string;

  private:
    std::array<float, 16u> elements_;
};

constexpr auto operator*=(Matrix4 &m1, const Matrix4 &m2) -> Matrix4 &
{
    auto result = Matrix4{};
    for (auto i = 0u; i < 4u; ++i)
    {
        for (auto j = 0u; j < 4u; ++j)
        {
            auto sum = 0.0f;
            for (auto k = 0u; k < 4u; ++k)
            {
                sum += m1.elements_[i + k * 4] * m2.elements_[k + j * 4];
            }
            result.elements_[i + j * 4] = sum;
        }
    }

    m1 = result;
    return m1;
}

constexpr auto operator*(const Matrix4 &m1, const Matrix4 &m2) -> Matrix4
{
    auto tmp{m1};
    return tmp *= m2;
}

inline constexpr auto Matrix4::look_at(const Vector3 &eye, const Vector3 &look_at, const Vector3 &up) -> Matrix4
{
    const auto f = Vector3::normalise(look_at - eye);
    const auto up_normalised = Vector3::normalise(up);

    const auto s = Vector3::normalise(Vector3::cross(f, up_normalised));
    const auto u = Vector3::normalise(Vector3::cross(s, f));

    auto m = Matrix4{};
    m.elements_ = {{s.x, u.x, -f.x, 0.0f, s.y, u.y, -f.y, 0.0f, s.z, u.z, -f.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};

    return m * Matrix4{-eye};
}

inline constexpr auto Matrix4::perspective(float fov, float width, float height, float near_plane, float far_plane)
    -> Matrix4
{
    Matrix4 m;

    const auto aspect_ratio = width / height;
    const auto tmp = std::tan(fov / 2.0f);
    const auto t = tmp * near_plane;
    const auto b = -t;
    const auto r = t * aspect_ratio;
    const auto l = b * aspect_ratio;

    m.elements_ = {
        {(2.0f * near_plane) / (r - l),
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         (2.0f * near_plane) / (t - b),
         0.0f,
         0.0f,
         (r + l) / (r - l),
         (t + b) / (t - b),
         -(far_plane + near_plane) / (far_plane - near_plane),
         -1.0f,
         0.0f,
         0.0f,
         -(2.0f * far_plane * near_plane) / (far_plane - near_plane),
         0.0f}};
    return m;
}

inline constexpr auto Matrix4::orthographic(float width, float height, float depth) -> Matrix4
{
    const auto right = width / 2.0f;
    const auto left = -right;
    const auto top = height / 2.0f;
    const auto bottom = -top;
    const auto far_p = depth;
    const auto near_p = 0.0f;

    auto m = Matrix4{};
    m.elements_ = {
        {2.0f / (right - left),
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         2.0f / (top - bottom),
         0.0f,
         0.0f,
         0.0f,
         0.0f,
         -2.0f / (far_p - near_p),
         0.0f,
         -(right + left) / (right - left),
         -(top + bottom) / (top - bottom),
         -(far_p + near_p) / (far_p - near_p),
         1.0f}};

    return m;
}

inline auto Matrix4::to_string() const -> std::string
{
    const auto *d = data().data();
    return std::format(
        "{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}",
        d[0],
        d[4],
        d[8],
        d[12],
        d[1],
        d[5],
        d[9],
        d[13],
        d[2],
        d[6],
        d[10],
        d[14],
        d[3],
        d[7],
        d[11],
        d[15]);
}
}
