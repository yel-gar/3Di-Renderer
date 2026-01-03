#include "Transform.hpp"

#include "math/MatrixTransforms.hpp"

#include <iostream>
#include <ostream>

namespace di_renderer::math {
    // rotation в радианах!
    Transform::Transform() : m_translation(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f), m_scale(1.0f, 1.0f, 1.0f) {}

    void Transform::set_position(const math::Vector3& position) {
        std::cout << position.x << " " << position.y << " " << position.z << '\n';
        m_translation = position;
    }

    void Transform::set_rotation(const math::Vector3& rotation) {
        m_rotation = rotation;
    }

    void Transform::set_scale(const math::Vector3& scale) {
        m_scale = scale;
    }

    void Transform::translate(const math::Vector3& translation) {
        m_translation += translation;
    }

    void Transform::scale(const math::Vector3& scale) {
        m_scale.x *= scale.x;
        m_scale.y *= scale.y;
        m_scale.z *= scale.z;
    }

    void Transform::rotate(const math::Vector3& rotation) {
        m_rotation += rotation;
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

    math::Matrix4x4 Transform::get_matrix() const {

        const auto t = math::MatrixTransforms::translate(m_translation);
        const auto s = math::MatrixTransforms::scale(m_scale);

        const auto rx = math::MatrixTransforms::rotate_x(m_rotation.x);
        const auto ry = math::MatrixTransforms::rotate_y(m_rotation.y);
        const auto rz = math::MatrixTransforms::rotate_z(m_rotation.z);

        // Комбинация вращения
        const auto r = rz * ry * rx;

        return t * r * s;
    }
} // namespace di_renderer::math
