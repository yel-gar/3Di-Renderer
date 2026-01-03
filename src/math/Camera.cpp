#include "Camera.hpp"

#include "math/MatrixTransforms.hpp"

#include <cmath>
#include <iostream>

namespace di_renderer::math {
    Camera::Camera()
        : m_position(Vector3(0, 0, 0)), m_target(Vector3(0, 0, 0)), m_fov(M_PI), m_aspect_ratio(1), m_near_plane(1),
          m_far_plane(10) {}

    Camera::Camera(const Vector3& position, const Vector3& target, float fov, float aspect_ratio, float near_plane,
                   float far_plane)
        : m_position(position), m_target(target), m_up(0.0f, 1.0f, 0.0f), m_fov(fov), m_aspect_ratio(aspect_ratio),
          m_near_plane(near_plane), m_far_plane(far_plane) {}

    void Camera::set_position(const Vector3& position) {
        m_position = position;
    }

    void Camera::set_target(const Vector3& target) {
        m_target = target;
    }

    void Camera::set_up_vector(const Vector3& up) {
        m_up = up;
    }

    void Camera::set_planes(const float near_plane, const float far_plane) {
        m_near_plane = near_plane;
        m_far_plane = far_plane;
    }

    void Camera::set_fov(const float fov) {
        m_fov = fov;
    }

    void Camera::set_aspect_ratio(const float aspect_ratio) {
        m_aspect_ratio = aspect_ratio;
    }

    const Vector3& Camera::get_position() const {
        return m_position;
    }

    const Vector3& Camera::get_target() const {
        return m_target;
    }

    void Camera::move_position(const Vector3& position) {
        m_position += position;
    }

    void Camera::move_target(const Vector3& target) {
        m_target += target;
    }

    void Camera::move(const Vector3& direction) {
        // TODO: remove this debug statement
        std::cout << "Translating camera: " << direction.x << ", " << direction.y << ", " << direction.z << '\n';
        m_position += direction;
        m_target += direction;
    }

    void Camera::parse_mouse_movement(double dx, double dy) {
        std::cout << "Moving camera: " << dx << ", " << dy << '\n';
    }

    Matrix4x4 Camera::get_view_matrix() const {
        return MatrixTransforms::look_at(m_position, m_target, m_up);
    }

    Matrix4x4 Camera::get_projection_matrix() const {
        return MatrixTransforms::perspective(m_fov, m_aspect_ratio, m_near_plane, m_far_plane);
    }

} // namespace di_renderer::math
