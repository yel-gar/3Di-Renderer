#include "Camera.hpp"

#include "math/MatrixTransforms.hpp"

namespace di_renderer::render {
    Camera::Camera(Vector3 position, Vector3 target, float fov, float aspectRatio, float nearPlane, float farPlane)
        : m_position(position), m_target(target), m_up(0.0F, 1.0F, 0.0F), m_fov(fov), m_aspect_ratio(aspectRatio),
          m_near_plane(nearPlane), m_far_plane(farPlane), m_view_changed(true), m_projection_changed(true) {}

    void Camera::set_position(const Vector3 position) {
        m_position = position;
        m_view_changed = true;
    }

    void Camera::set_target(const Vector3 target) {
        m_target = target;
        m_view_changed = true;
    }

    void Camera::set_up_vector(const Vector3 up) {
        m_up = up;
        m_view_changed = true;
    }

    void Camera::set_planes(const float nearPlane, const float farPlane) {
        m_near_plane = nearPlane;
        m_far_plane = farPlane;
    }

    void Camera::set_fov(const float fov) {
        m_fov = fov;
        m_projection_changed = true;
    }

    void Camera::set_aspect_ratio(const float aspectRatio) {
        m_aspect_ratio = aspectRatio;
        m_projection_changed = true;
    }

    Vector3 Camera::get_position() const {
        return m_position;
    }

    Vector3 Camera::get_target() const {
        return m_target;
    }

    void Camera::move_position(const Vector3 position) {
        m_position += position;
        m_projection_changed = true;
    }

    void Camera::move_target(const Vector3 target) {
        m_target += target;
        m_projection_changed = true;
    }

    void Camera::move(Vector3 direction) {
        m_position += direction;
        m_target += direction;
        m_projection_changed = true;
    }

    Matrix4x4 Camera::get_view_matrix() const {
        if (m_view_changed) {
            m_view_matrix = MatrixTransforms::look_at(m_position, m_target, Vector3(0.0F, 1.0F, 0.0F));
            m_view_changed = false;
        }
        return m_view_matrix;
    }

    Matrix4x4 Camera::get_projection_matrix() const {
        if (m_projection_changed) {
            m_projection_matrix = MatrixTransforms::perspective(m_fov, m_aspect_ratio, m_near_plane, m_far_plane);
            m_projection_changed = false;
        }
        return m_projection_matrix;
    }

} // namespace di_renderer::render
