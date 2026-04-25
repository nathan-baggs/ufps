#pragma once

#include <array>
#include <span>

#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "utils/data_buffer.h"

namespace ufps
{

namespace impl
{

constexpr auto create_direction(float pitch, float yaw) -> ufps::Vector3
{
    return ufps::Vector3::normalise(
        {std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)});
}
}

struct CameraData
{
    Matrix4 view;
    Matrix4 projection;
    Vector3 position;
    float pad = 0.0f;
};

class Camera
{
  public:
    constexpr Camera(
        const Vector3 &position,
        const Vector3 &look_at,
        const Vector3 &up,
        float fov,
        float width,
        float height,
        float near_plane,
        float far_plane);

    constexpr Camera(float width, float height, float depth);

    constexpr auto position() const -> Vector3;
    constexpr auto set_position(const Vector3 &position) -> void;
    constexpr auto direction() const -> Vector3;
    constexpr auto up() const -> Vector3;
    constexpr auto right() const -> Vector3;
    constexpr auto adjust_yaw(float adjust) -> void;
    constexpr auto adjust_pitch(float adjust) -> void;
    constexpr auto set_yaw(float yaw) -> void;
    constexpr auto set_pitch(float pitch) -> void;
    constexpr auto translate(const Vector3 &translation) -> void;
    constexpr auto fov() const -> float;
    constexpr auto width() const -> float;
    constexpr auto height() const -> float;
    constexpr auto near_plane() const -> float;
    constexpr auto far_plane() const -> float;
    constexpr auto data() const -> const CameraData &;
    constexpr auto data_view() const -> DataBufferView;

  private:
    CameraData data_;
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

constexpr Camera::Camera(
    const Vector3 &position,
    const Vector3 &look_at,
    const Vector3 &up,
    float fov,
    float width,
    float height,
    float near_plane,
    float far_plane)
    : data_{
          .view = Matrix4::look_at(position, look_at, up),
          .projection = Matrix4::perspective(fov, width, height, near_plane, far_plane),
          .position = position}
    , direction_(look_at)
    , up_(up)
    , right_(Vector3::normalise(Vector3::cross(direction_, up_)))
    , pitch_{}
    , yaw_{-std::numbers::pi_v<float> / 2.0f}
    , fov_(fov)
    , width_(width)
    , height_(height)
    , near_plane_(near_plane)
    , far_plane_(far_plane)
{
    direction_ = impl::create_direction(pitch_, yaw_);
    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
    adjust_pitch(0.0f);
}

constexpr Camera::Camera(float width, float height, float depth)
    : data_{
          .view = Matrix4::look_at(Vector3{0.0f, 0.0f, 1.0f}, {}, {0.0f, 1.0f, 0.0f}),
          .projection = Matrix4::orthographic(width, height, depth),
          .position = {0.0f, 0.0f, 1.0f}}
    , direction_(Vector3{0.0f, 0.0f, -1.0f})
    , up_(Vector3{0.0f, 1.0f, 0.0f})
    , right_(Vector3::normalise(Vector3::cross(direction_, up_)))
    , pitch_{}
    , yaw_{-std::numbers::pi_v<float> / 2.0f}
    , fov_(0.0f)
    , width_(width)
    , height_(height)
    , near_plane_(0.0f)
    , far_plane_(depth)
{
}

constexpr auto Camera::position() const -> Vector3
{
    return data_.position;
}

constexpr auto Camera::set_position(const Vector3 &position) -> void
{
    data_.position = position;
    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
}

constexpr auto Camera::direction() const -> Vector3
{
    return direction_;
}

constexpr auto Camera::up() const -> Vector3
{
    return up_;
}

constexpr auto Camera::right() const -> Vector3
{
    return right_;
}

constexpr auto Camera::adjust_yaw(float adjust) -> void
{
    yaw_ += adjust;
    direction_ = impl::create_direction(pitch_, yaw_);

    const auto world_up = Vector3{0.0f, 1.0f, 0.0f};
    right_ = Vector3::normalise(Vector3::cross(direction_, world_up));
    up_ = Vector3::normalise(Vector3::cross(right_, direction_));

    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
}

constexpr auto Camera::adjust_pitch(float adjust) -> void
{
    pitch_ += adjust;
    constexpr auto pitch_epsilon = 0.0001f;
    pitch_ = std::clamp(
        pitch_,
        (-std::numbers::pi_v<float> / 2.0f) + pitch_epsilon,
        (std::numbers::pi_v<float> / 2.0f) - pitch_epsilon);
    direction_ = impl::create_direction(pitch_, yaw_);

    const auto world_up = Vector3{0.0f, 1.0f, 0.0f};
    right_ = Vector3::normalise(Vector3::cross(direction_, world_up));
    up_ = Vector3::normalise(Vector3::cross(right_, direction_));

    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
}

constexpr auto Camera::set_yaw(float yaw) -> void
{
    adjust_yaw(yaw - yaw_);
}

constexpr auto Camera::set_pitch(float pitch) -> void
{
    adjust_pitch(pitch - pitch_);
}

constexpr auto Camera::translate(const Vector3 &translation) -> void
{
    data_.position += translation;
    direction_ = impl::create_direction(pitch_, yaw_);
    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
}

constexpr auto Camera::fov() const -> float
{
    return fov_;
}

constexpr auto Camera::width() const -> float
{
    return width_;
}

constexpr auto Camera::height() const -> float
{
    return height_;
}

constexpr auto Camera::near_plane() const -> float
{
    return near_plane_;
}

constexpr auto Camera::far_plane() const -> float
{
    return far_plane_;
}

constexpr auto Camera::data() const -> const CameraData &
{
    return data_;
}

constexpr auto Camera::data_view() const -> DataBufferView
{
    return {reinterpret_cast<const std::byte *>(&data_), sizeof(data_)};
}

}
