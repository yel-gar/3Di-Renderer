#include "Transform.hpp"

namespace di_renderer::core {
    // rotation в радианах!
    Transform::Transform()
        : m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f),
          m_matrix(math::Matrix4x4::identity()), m_is_changed(false) {}

    void Transform::set_position(const math::Vector3& position) {
        m_position = position;
        m_is_changed = true;
    }

    void Transform::set_rotation(const math::Vector3& rotation) {
        m_rotation = rotation;
        m_is_changed = true;
    }

    void Transform::set_scale(const math::Vector3& scale) {
        m_scale = scale;
        m_is_changed = true;
    }

    void Transform::translate(const math::Vector3& translation) {
        m_position += translation;
        m_is_changed = true;
    }

    void Transform::scale(const math::Vector3& scale) {
        m_scale.x *= scale.x;
        m_scale.y *= scale.y;
        m_scale.z *= scale.z;
        m_is_changed = true;
    }

    void Transform::rotate(const math::Vector3& rotation) {
        m_rotation += rotation;
        m_is_changed = true;
    }

    const math::Vector3& Transform::get_position() const {
        return m_position;
    }

    const math::Vector3& Transform::get_rotation() const {
        return m_rotation;
    }

    const math::Vector3& Transform::get_scale() const {
        return m_scale;
    }

    const math::Matrix4x4& Transform::get_matrix() const {
        if (m_is_changed) {
            recalculate_matrix();
        }
        return m_matrix;
    }

    void Transform::recalculate_matrix() const {
        auto t = math::MatrixTransforms::translate(m_position);
        auto s = math::MatrixTransforms::scale(m_scale);

        auto Rx = math::MatrixTransforms::rotate_x(m_rotation.x);
        auto Ry = math::MatrixTransforms::rotate_y(m_rotation.y);
        auto Rz = math::MatrixTransforms::rotate_z(m_rotation.z);

        // Комбинация вращения
        auto r = Rz * Ry * Rx;

        m_matrix = t * r * s;

        m_is_changed = false;
    }
} // namespace di_renderer::core
