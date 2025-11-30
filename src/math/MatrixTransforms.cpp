#include "MatrixTransforms.hpp"

#include <cmath>

namespace di_renderer::math {
    Matrix4x4 MatrixTransforms::translate(const Vector3& offset) {
        Matrix4x4 res = Matrix4x4::identity();

        res(0, 3) = offset.x;
        res(1, 3) = offset.y;
        res(2, 3) = offset.z;

        return res;
    }

    Matrix4x4 MatrixTransforms::scale(const Vector3& scale) {
        Matrix4x4 res;

        res(0, 0) = scale.x;
        res(1, 1) = scale.y;
        res(2, 2) = scale.z;
        res(3, 3) = 1.0F;

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_x(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        Matrix4x4 res = Matrix4x4::identity();

        res(1, 1) = c;
        res(1, 2) = -s;
        res(2, 1) = s;
        res(2, 2) = c;

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_y(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        Matrix4x4 res = Matrix4x4::identity();

        res(0, 0) = c;
        res(0, 2) = -s;
        res(2, 0) = s;
        res(2, 2) = c;

        return res;
    }
    Matrix4x4 MatrixTransforms::rotate_z(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        Matrix4x4 res = Matrix4x4::identity();

        res(0, 0) = c;
        res(0, 1) = -s;
        res(1, 0) = s;
        res(1, 1) = c;

        return res;
    }
} // namespace di_renderer::math
