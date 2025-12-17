#pragma once

#include <format>
#include <string>

#include "maths/matrix4.h"
#include "maths/quaternion.h"
#include "maths/vector3.h"
#include "utils/formatter.h"

namespace ufps
{

class Transform
{
  public:
    constexpr Transform(const Vector3 &position, const Vector3 &scale, const Quaternion &rotation)
        : position(position)
        , scale(scale)
        , rotation(rotation)
    {
    }

    constexpr Transform(const Matrix4 &transform)
        : position{}
        , scale{}
        , rotation{}
    {
        position = Vector3{transform[12], transform[13], transform[14]};
        scale = Vector3{
            Vector3{transform[0], transform[1], transform[2]}.length(),
            Vector3{transform[4], transform[5], transform[6]}.length(),
            Vector3{transform[8], transform[9], transform[10]}.length(),
        };

        auto trace = 0.0f;
        auto s = 0.0f;

        const auto scale_x =
            std::sqrt(transform[0] * transform[0] + transform[1] * transform[1] + transform[2] * transform[2]);
        const auto scale_y =
            std::sqrt(transform[4] * transform[4] + transform[5] * transform[5] + transform[6] * transform[6]);
        const auto scale_z =
            std::sqrt(transform[8] * transform[8] + transform[9] * transform[9] + transform[10] * transform[10]);

        const auto m00 = transform[0] / scale_x;
        const auto m10 = transform[1] / scale_x;
        const auto m20 = transform[2] / scale_x;

        const auto m01 = transform[4] / scale_y;
        const auto m11 = transform[5] / scale_y;
        const auto m21 = transform[6] / scale_y;

        const auto m02 = transform[8] / scale_z;
        const auto m12 = transform[9] / scale_z;
        const auto m22 = transform[10] / scale_z;

        trace = m00 + m11 + m22;

        if (trace > 0.0f)
        {
            s = 0.5f / std::sqrt(trace + 1.0f);
            rotation.w = 0.25f / s;
            rotation.x = (m21 - m12) * s;
            rotation.y = (m02 - m20) * s;
            rotation.z = (m10 - m01) * s;
        }
        else
        {
            if (m00 > m11 && m00 > m22)
            {
                s = 2.0f * std::sqrt(1.0f + m00 - m11 - m22);
                rotation.w = (m21 - m12) / s;
                rotation.x = 0.25f * s;
                rotation.y = (m01 + m10) / s;
                rotation.z = (m02 + m20) / s;
            }
            else if (m11 > m22)
            {
                s = 2.0f * std::sqrt(1.0f + m11 - m00 - m22);
                rotation.w = (m02 - m20) / s;
                rotation.x = (m01 + m10) / s;
                rotation.y = 0.25f * s;
                rotation.z = (m12 + m21) / s;
            }
            else
            {
                s = 2.0f * std::sqrt(1.0f + m22 - m00 - m11);
                rotation.w = (m10 - m01) / s;
                rotation.x = (m02 + m20) / s;
                rotation.y = (m12 + m21) / s;
                rotation.z = 0.25f * s;
            }
        }
    }

    constexpr operator Matrix4() const
    {
        return Matrix4{position} * Matrix4{rotation} * Matrix4{scale, Matrix4::Scale{}};
    }

    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
};

inline auto to_string(const Transform &transform) -> std::string
{
    return std::format("pos:{}, scale:{}, rot:{}", transform.position, transform.scale, transform.rotation);
}

};
