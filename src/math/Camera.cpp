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

    Vector3 Camera::get_front() const {
        return (m_target - m_position).normalized();
    }

    void Camera::move_position(const Vector3& position) {
        m_position += position;
    }

    void Camera::move_target(const Vector3& target) {
        m_target += target;
    }

    void Camera::move(const Vector3& direction) {
        m_position += direction;
        m_target += direction;
    }

    void Camera::rotate_view(double dx, double dy) {}

    void Camera::orbit_around_target(double dx, double dy) {}

    void Camera::zoom(double offset) {}

    void Camera::update_eulers_from_vectors() {
        Vector3 front = get_front();
        m_pitch = std::asin(front.y) * (180.0f / M_PIf);
        m_yaw = std::atan2(front.z, front.x) * (180.0f / M_PIf);
    }

    void Camera::update_vectors_from_euler(bool is_orbiting) {
        Vector3 front;
        float rad_yaw = m_yaw * (M_PIf / 180.0f);
        float rad_pitch = m_pitch * (M_PIf / 180.0f);

        front.x = std::cos(rad_yaw) * std::cos(rad_pitch);
        front.y = std::sin(rad_pitch);
        front.z = std::sin(rad_yaw) * std::cos(rad_pitch);

        if (is_orbiting) {
            Vector3 diff = m_position - m_target;
            float radius = std::sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));

            m_position = m_target - (front * radius);
        } else {
            float view_distance = 1.0f;
            m_target = m_position + (front * view_distance);
        }
    }
    Matrix4x4 Camera::get_view_matrix() const {
        return MatrixTransforms::look_at(m_position, m_target, m_up);
    }

    Matrix4x4 Camera::get_projection_matrix() const {
        return MatrixTransforms::perspective(m_fov, m_aspect_ratio, m_near_plane, m_far_plane);
    }

} // namespace di_renderer::math
