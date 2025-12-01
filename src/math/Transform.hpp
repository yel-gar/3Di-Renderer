#pragma once

#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::core {
    class Transform {
      public:
        Transform();

        void set_position(const math::Vector3& position);
        void set_rotation(const math::Vector3& rotation);
        void set_scale(const math::Vector3& scale);

        void translate(const math::Vector3& translation);
        void rotate(const math::Vector3& rotation);
        void scale(const math::Vector3& scale);

        const math::Vector3& get_position() const;
        const math::Vector3& get_scale() const;
        const math::Vector3& get_rotation() const;

        const math::Matrix4x4& get_matrix() const;

      private:
        math::Vector3 m_translation;
        math::Vector3 m_rotation;
        math::Vector3 m_scale;

        mutable math::Matrix4x4 m_matrix;
        mutable bool m_is_changed{false};

        void recalculate_matrix() const;
    };
} // namespace di_renderer::core
