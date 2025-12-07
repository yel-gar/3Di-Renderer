#include "Transform.hpp"

#include "math/MatrixTransforms.hpp"

namespace di_renderer::math {
    // rotation в радианах!
    Transform::Transform()
        : m_translation(0.0F, 0.0F, 0.0F), m_rotation(0.0F, 0.0F, 0.0F), m_scale(1.0F, 1.0F, 1.0F),
          m_matrix(math::Matrix4x4::identity()) {}

    void Transform::set_position(const math::Vector3& position) {
        m_translation = position;
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
        m_translation += translation;
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
        return m_translation;
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
        auto t = math::MatrixTransforms::translate(m_translation);
        auto s = math::MatrixTransforms::scale(m_scale);

        auto rx = math::MatrixTransforms::rotate_x(m_rotation.x);
        auto ry = math::MatrixTransforms::rotate_y(m_rotation.y);
        auto rz = math::MatrixTransforms::rotate_z(m_rotation.z);

        // Комбинация вращения
        auto r = rz * ry * rx;

        m_matrix = t * r * s;

        m_is_changed = false;
    }
} // namespace di_renderer::math
