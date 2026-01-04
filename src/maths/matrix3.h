#pragma once

#include <array>
#include <format>
#include <ranges>
#include <span>

#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "utils/error.h"

namespace ufps
{

class Matrix3
{
  public:
    constexpr Matrix3()
        : elements_({
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
              0.0f,
              0.0f,
              0.0f,
              1.0f,
          })
    {
    }

    constexpr Matrix3(const std::array<float, 9u> &elements)
        : elements_(elements)
    {
    }

    constexpr Matrix3(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)
        : elements_({
              v1.x,
              v1.y,
              v1.z,
              v2.x,
              v2.y,
              v2.z,
              v3.x,
              v3.y,
              v3.z,
          })
    {
    }

    constexpr Matrix3(const Matrix4 &m4)
        : elements_({
              m4[0],
              m4[1],
              m4[2],
              m4[4],
              m4[5],
              m4[6],
              m4[8],
              m4[9],
              m4[10],
          })
    {
    }

    constexpr auto data() const -> std::span<const float>
    {
        return elements_;
    }

    constexpr auto operator[](this auto &&self, std::size_t index) -> auto &
    {
        return self.elements_[index];
    }

    auto row(std::size_t index) const -> Vector3
    {
        ensure(index <= 2, "index out of range");

        return {elements_[index], elements_[index + 3u], elements_[index + 6u]};
    }

    friend constexpr auto operator*=(Matrix3 &m1, const Matrix3 &m2) -> Matrix3 &;

    static constexpr auto invert(const Matrix3 &m) -> Matrix3;

    static constexpr auto transpose(Matrix3 m) -> Matrix3;

    constexpr auto operator==(const Matrix3 &) const -> bool = default;

    auto to_string() const -> std::string;

  private:
    std::array<float, 9u> elements_;
};

constexpr auto operator*=(Matrix3 &m1, const Matrix3 &m2) -> Matrix3 &
{
    auto result = Matrix3{};
    for (auto i = 0u; i < 3u; ++i)
    {
        for (auto j = 0u; j < 3u; ++j)
        {
            result.elements_[i + j * 3] = 0.0f;
            for (auto k = 0u; k < 3u; ++k)
            {
                result.elements_[i + j * 3] += m1.elements_[i + k * 3] * m2.elements_[k + j * 3];
            }
        }
    }

    m1 = result;
    return m1;
}

constexpr auto operator*(const Matrix3 &m1, const Matrix3 &m2) -> Matrix3
{
    auto tmp{m1};
    return tmp *= m2;
}

constexpr auto operator*(const Matrix3 &m, const Vector3 &v) -> Vector3
{
    return {
        m[0] * v.x + m[3] * v.y + m[6] * v.z,
        m[1] * v.x + m[4] * v.y + m[7] * v.z,
        m[2] * v.x + m[5] * v.y + m[8] * v.z};
}

inline auto Matrix3::to_string() const -> std::string
{
    const auto *d = data().data();
    return std::format("{} {} {}\n{} {} {}\n{} {} {}", d[0], d[3], d[6], d[1], d[4], d[7], d[2], d[5], d[8]);
}

constexpr auto Matrix3::invert(const Matrix3 &m) -> Matrix3
{
    const auto adjoint = Matrix3{{
        (m[4] * m[8]) - (m[5] * m[7]),
        -((m[1] * m[8]) - (m[2] * m[7])),
        (m[1] * m[5]) - (m[2] * m[4]),
        -((m[3] * m[8]) - (m[5] * m[6])),
        (m[0] * m[8]) - (m[2] * m[6]),
        -((m[0] * m[5]) - (m[2] * m[3])),
        (m[3] * m[7]) - (m[4] * m[6]),
        -((m[0] * m[7]) - (m[1] * m[6])),
        (m[0] * m[4]) - (m[1] * m[3]),
    }};

    const auto determinant = (m[0] * m[4] * m[8]) + (m[3] * m[7] * m[2]) + (m[6] * m[1] * m[5]) - (m[0] * m[7] * m[5]) -
                             (m[3] * m[1] * m[8]) - (m[6] * m[4] * m[2]);

    const auto inverted_values =
        adjoint.data() | std::views::transform([determinant](auto e) { return e / determinant; });

    auto inverted_array = std::array<float, 9u>{};
    std::ranges::copy(inverted_values, std::ranges::begin(inverted_array));

    return {inverted_array};
}

constexpr auto Matrix3::transpose(Matrix3 m) -> Matrix3
{
    std::ranges::swap(m[1], m[3]);
    std::ranges::swap(m[2], m[6]);
    std::ranges::swap(m[5], m[7]);

    return m;
}
}
