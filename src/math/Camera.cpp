#include "Camera.hpp"

#include "math/MatrixTransforms.hpp"

#include <algorithm>
#include <cmath>

namespace di_renderer::math {
    Camera::Camera()
        : m_position(0.0f, 0.0f, 3.0f), m_target(0.0f, 0.0f, 0.0f), m_fov(45.0f * static_cast<float>(M_PI) / 180.0f),
          m_aspect_ratio(1.0f), m_near_plane(0.1f), m_far_plane(1000.0f) {
        update_euler_from_vectors();
    }

    Camera::Camera(const Vector3& position, const Vector3& target, float fov, float aspect_ratio, float near_plane,
                   float far_plane)
        : m_position(position), m_target(target), m_up(0.0f, 1.0f, 0.0f), m_fov(fov), m_aspect_ratio(aspect_ratio),
          m_near_plane(near_plane), m_far_plane(far_plane) {

        update_euler_from_vectors();
    }
    void Camera::set_position(const Vector3& position) {
        m_position = position;
        update_euler_from_vectors();
    }

    void Camera::set_target(const Vector3& target) {
        m_target = target;
        update_euler_from_vectors();
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
        update_euler_from_vectors();
    }

    void Camera::move_target(const Vector3& target) {
        m_target += target;
        update_euler_from_vectors();
    }

    void Camera::move(const Vector3& direction) {
        m_position += direction;
        m_target += direction;
    }

    void Camera::rotate_view(float dx, float dy) {
        m_yaw -= dx * m_sensitivity;
        m_pitch += dy * m_sensitivity;
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        update_vectors_from_euler(false);
    }

    void Camera::orbit_around_target(float dx, float dy) {
        m_yaw -= dx * m_sensitivity;
        m_pitch += dy * m_sensitivity;
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        update_vectors_from_euler(true);
    }

    void Camera::zoom(float offset) {
        const float zoom_amount = offset * m_zoom_speed;
        const Vector3 move_vector = get_front() * zoom_amount;
        const Vector3 vec_to_target = m_target - m_position;
        const float dist = std::sqrt((vec_to_target.x * vec_to_target.x) + (vec_to_target.y * vec_to_target.y) +
                                     (vec_to_target.z * vec_to_target.z));
        if (zoom_amount < dist - 0.1f) {
            m_position += move_vector;
        }
    }

    void Camera::update_euler_from_vectors() {
        const Vector3 front = get_front();
        m_pitch = std::asin(front.y) * (180.0f / M_PIf);
        m_yaw = std::atan2(front.z, front.x) * (180.0f / M_PIf);
    }

    void Camera::update_vectors_from_euler(bool is_orbiting) {
        Vector3 front;
        const float rad_yaw = m_yaw * (M_PIf / 180.0f);
        const float rad_pitch = m_pitch * (M_PIf / 180.0f);

        front.x = std::cos(rad_yaw) * std::cos(rad_pitch);
        front.y = std::sin(rad_pitch);
        front.z = std::sin(rad_yaw) * std::cos(rad_pitch);

        if (is_orbiting) {
            const Vector3 diff = m_position - m_target;
            const float radius = std::sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));

            m_position = m_target - (front * radius);
        } else {
            const float view_distance = 1.0f;
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
