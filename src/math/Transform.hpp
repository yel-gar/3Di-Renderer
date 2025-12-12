#pragma once

#include "math/Matrix4x4.hpp"
#include "math/Vector3.hpp"

namespace di_renderer::math {
    class Transform {
      public:
        Transform();

        void set_position(const Vector3& position);
        void set_rotation(const Vector3& rotation);
        void set_scale(const Vector3& scale);

        void translate(const Vector3& translation);
        void rotate(const Vector3& rotation);
        void scale(const Vector3& scale);

        const Vector3& get_position() const;
        const Vector3& get_scale() const;
        const Vector3& get_rotation() const;

        const Matrix4x4& get_matrix() const;

      private:
        Vector3 m_translation;
        Vector3 m_rotation;
        Vector3 m_scale;

        mutable Matrix4x4 m_matrix;
        mutable bool m_is_changed{false};

        void recalculate_matrix() const;
    };
} // namespace di_renderer::math
