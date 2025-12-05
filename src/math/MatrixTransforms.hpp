#pragma once

#include "Matrix4x4.hpp"
#include "Vector3.hpp"

namespace di_renderer::math {
    class MatrixTransforms {
      public:
        static Matrix4x4 translate(const Vector3& offset);

        static Matrix4x4 scale(const Vector3& scale);

        static Matrix4x4 rotate_x(float angle);
        static Matrix4x4 rotate_y(float angle);
        static Matrix4x4 rotate_z(float angle);
    };
} // namespace di_renderer::math
