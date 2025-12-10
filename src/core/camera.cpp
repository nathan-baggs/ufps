#include "camera.h"

#include <cmath>
#include <numbers>
#include <span>

#include "maths/matrix4.h"
#include "maths/vector3.h"
#include "utils/data_buffer.h"

namespace
{

constexpr auto create_direction(float pitch, float yaw) -> ufps::Vector3
{
    return ufps::Vector3::normalise(
        {std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)});
}

}

namespace ufps
{

Camera::Camera(
    const Vector3 &position,
    const Vector3 &look_at,
    const Vector3 &up,
    float fov,
    float width,
    float height,
    float near_plane,
    float far_plane)
    : data_{.view = Matrix4::look_at(position, look_at, up), .projection = Matrix4::perspective(fov, width, height, near_plane, far_plane)}
    , position_(position)
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
    direction_ = create_direction(pitch_, yaw_);
    data_.view = Matrix4::look_at(position_, position_ + direction_, up_);
    adjust_pitch(0.0f);
}

Camera::Camera(float width, float height, float depth)
    : data_{.view = Matrix4::look_at(Vector3{0.0f, 0.0f, 1.0f}, {}, {0.0f, 1.0f, 0.0f}), .projection = Matrix4::orthographic(width, height, depth),}
    , position_(Vector3{0.0f, 0.0f, 1.0f})
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

auto Camera::position() const -> Vector3
{
    return position_;
}

auto Camera::set_position(const Vector3 &position) -> void
{
    position_ = position;
    data_.view = Matrix4::look_at(position_, position_ + direction_, up_);
}

auto Camera::direction() const -> Vector3
{
    return direction_;
}

auto Camera::up() const -> Vector3
{
    return up_;
}

auto Camera::right() const -> Vector3
{
    return right_;
}

auto Camera::adjust_yaw(float adjust) -> void
{
    yaw_ += adjust;
    direction_ = create_direction(pitch_, yaw_);

    const auto world_up = Vector3{0.0f, 1.0f, 0.0f};
    right_ = Vector3::normalise(Vector3::cross(direction_, world_up));
    up_ = Vector3::normalise(Vector3::cross(right_, direction_));

    data_.view = Matrix4::look_at(position_, position_ + direction_, up_);
}

auto Camera::adjust_pitch(float adjust) -> void
{
    pitch_ += adjust;
    constexpr auto pitch_epsilon = 0.0001f;
    pitch_ = std::clamp(
        pitch_,
        (-std::numbers::pi_v<float> / 2.0f) + pitch_epsilon,
        (std::numbers::pi_v<float> / 2.0f) - pitch_epsilon);
    direction_ = create_direction(pitch_, yaw_);

    const auto world_up = Vector3{0.0f, 1.0f, 0.0f};
    right_ = Vector3::normalise(Vector3::cross(direction_, world_up));
    up_ = Vector3::normalise(Vector3::cross(right_, direction_));

    data_.view = Matrix4::look_at(position_, position_ + direction_, up_);
}

auto Camera::set_yaw(float yaw) -> void
{
    adjust_yaw(yaw - yaw_);
}

auto Camera::set_pitch(float pitch) -> void
{
    adjust_pitch(pitch - pitch_);
}

auto Camera::translate(const Vector3 &translation) -> void
{
    position_ += translation;
    direction_ = create_direction(pitch_, yaw_);
    data_.view = Matrix4::look_at(position_, position_ + direction_, up_);
}

auto Camera::fov() const -> float
{
    return fov_;
}

auto Camera::width() const -> float
{
    return width_;
}

auto Camera::height() const -> float
{
    return height_;
}

auto Camera::near_plane() const -> float
{
    return near_plane_;
}

auto Camera::far_plane() const -> float
{
    return far_plane_;
}

auto Camera::data() const -> const CameraData &
{
    return data_;
}

auto Camera::data_view() const -> DataBufferView
{
    return {reinterpret_cast<const std::byte *>(&data_), sizeof(data_)};
}
}
