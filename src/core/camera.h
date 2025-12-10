#pragma once

#include <array>
#include <span>

#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "utils/data_buffer.h"

namespace ufps
{

struct CameraData
{
    Matrix4 view;
    Matrix4 projection;
};

class Camera
{
  public:
    Camera(
        const Vector3 &position,
        const Vector3 &look_at,
        const Vector3 &up,
        float fov,
        float width,
        float height,
        float near_plane,
        float far_plane);

    Camera(float width, float height, float depth);

    auto position() const -> Vector3;
    auto set_position(const Vector3 &position) -> void;
    auto direction() const -> Vector3;
    auto up() const -> Vector3;
    auto right() const -> Vector3;
    auto adjust_yaw(float adjust) -> void;
    auto adjust_pitch(float adjust) -> void;
    auto set_yaw(float yaw) -> void;
    auto set_pitch(float pitch) -> void;
    auto translate(const Vector3 &translation) -> void;
    auto fov() const -> float;
    auto width() const -> float;
    auto height() const -> float;
    auto near_plane() const -> float;
    auto far_plane() const -> float;
    auto data() const -> const CameraData &;
    auto data_view() const -> DataBufferView;

  private:
    CameraData data_;
    Vector3 position_;
    Vector3 direction_;
    Vector3 up_;
    Vector3 right_;
    float pitch_;
    float yaw_;
    float fov_;
    float width_;
    float height_;
    float near_plane_;
    float far_plane_;
};

}
