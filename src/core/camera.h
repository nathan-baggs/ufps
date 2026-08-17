#pragma once

#include <array>
#include <span>

#include "maths/matrix4.h"
#include "maths/transform.h"
#include "maths/vector3.h"
#include "utils/data_buffer.h"

namespace ufps
{

namespace impl
{

constexpr auto extract_up(const ufps::Transform &transform) -> ufps::Vector3
{
    const auto m = ufps::Matrix4{transform};
    return ufps::Vector3::normalise({m[4zu], m[5zu], m[6zu]});
}

constexpr auto extract_right(const ufps::Transform &transform) -> ufps::Vector3
{
    const auto m = ufps::Matrix4{transform};
    return -ufps::Vector3::normalise({m[0zu], m[1zu], m[2zu]});
}

constexpr auto extract_direction(const ufps::Transform &transform) -> ufps::Vector3
{
    const auto m = ufps::Matrix4{transform};
    return ufps::Vector3::normalise({m[8zu], m[9zu], m[10zu]});
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
    struct Description
    {
        Transform transform;
        float fov;
        float width;
        float height;
        float near_plane;
        float far_plane;
    };

    constexpr Camera(const Description &description);

    constexpr Camera(
        const Transform &transform,
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
    constexpr auto transform() const -> const Transform &;
    constexpr auto set_transform(const Transform &transform) -> void;
    constexpr auto parent_transform() const -> const Transform &;
    constexpr auto set_parent_transform(const Transform &transform) -> void;
    constexpr auto local_transform() const -> const Transform &;
    constexpr auto fov() const -> float;
    constexpr auto width() const -> float;
    constexpr auto height() const -> float;
    constexpr auto near_plane() const -> float;
    constexpr auto far_plane() const -> float;
    constexpr auto data() const -> const CameraData &;
    constexpr auto data_view() const -> DataBufferView;
    constexpr auto description() const -> Description;

  private:
    constexpr auto update_transforms(const Transform &local, const Transform &parent) -> void;

    Vector3 direction_;
    Vector3 up_;
    Vector3 right_;
    Transform local_transform_;
    Transform parent_transform_;
    Transform transform_;
    float fov_;
    float width_;
    float height_;
    float near_plane_;
    float far_plane_;
    CameraData data_;
};

constexpr Camera::Camera(const Description &description)
    : Camera{
          description.transform,
          description.fov,
          description.width,
          description.height,
          description.near_plane,
          description.far_plane}
{
}

constexpr Camera::Camera(
    const Transform &transform,
    float fov,
    float width,
    float height,
    float near_plane,
    float far_plane)
    : direction_(impl::extract_direction(transform))
    , up_(impl::extract_up(transform))
    , right_(impl::extract_right(transform))
    , local_transform_{transform}
    , parent_transform_{}
    , transform_{transform}
    , fov_(fov)
    , width_(width)
    , height_(height)
    , near_plane_(near_plane)
    , far_plane_(far_plane)
    , data_{
          .view = Matrix4::look_at(transform.position, direction_, up_),
          .projection = Matrix4::perspective(fov, width, height, near_plane, far_plane),
          .position = transform.position}
{
}

constexpr Camera::Camera(float width, float height, float depth)
    : direction_(Vector3{0.0f, 0.0f, -1.0f})
    , up_(Vector3{0.0f, 1.0f, 0.0f})
    , right_(Vector3::normalise(Vector3::cross(direction_, up_)))
    , fov_(0.0f)
    , width_(width)
    , height_(height)
    , near_plane_(0.0f)
    , far_plane_(depth)
    , data_{
          .view = Matrix4::look_at(Vector3{0.0f, 0.0f, 1.0f}, {}, {0.0f, 1.0f, 0.0f}),
          .projection = Matrix4::orthographic(width, height, depth),
          .position = {0.0f, 0.0f, 1.0f}}
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

constexpr auto Camera::transform() const -> const Transform &
{
    return transform_;
}

constexpr auto Camera::set_transform(const Transform &transform) -> void
{
    update_transforms(transform, parent_transform_);
}

constexpr auto Camera::parent_transform() const -> const Transform &
{
    return parent_transform_;
}

constexpr auto Camera::local_transform() const -> const Transform &
{
    return local_transform_;
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

constexpr auto Camera::update_transforms(const Transform &local, const Transform &parent) -> void
{
    transform_ = parent * local;
    local_transform_ = local;
    parent_transform_ = parent;

    direction_ = impl::extract_direction(transform_);
    up_ = impl::extract_up(transform_);
    right_ = impl::extract_right(transform_);
    data_.position = transform_.position;
    data_.view = Matrix4::look_at(data_.position, data_.position + direction_, up_);
}

constexpr auto Camera::set_parent_transform(const Transform &transform) -> void
{
    update_transforms(local_transform_, transform);
}

constexpr auto Camera::description() const -> Description
{
    return {
        .transform = local_transform_,
        .fov = fov_,
        .width = width_,
        .height = height_,
        .near_plane = near_plane_,
        .far_plane = far_plane_,
    };
}

}
