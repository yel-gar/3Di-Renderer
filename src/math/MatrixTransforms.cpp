#include "MatrixTransforms.hpp"

#include <cmath>

namespace di_renderer::math {
    Matrix4x4 MatrixTransforms::translate(const Vector3& offset) {
        // clang-format off
        Matrix4x4 res({
            1, 0, 0, offset.x,
            0 ,1, 0, offset.y,
            0, 0, 1, offset.z,
            0, 0, 0, 1});
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::scale(const Vector3& scale) {
        // clang-format off
        Matrix4x4 res({
            scale.x, 0, 0, 0,
            0, scale.y, 0, 0,
            0, 0, scale.z, 0,
            0, 0, 0, 1});
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_x(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        // clang-format off
        Matrix4x4 res({
            1, 0, 0, 0,
            0, c, -s, 0,
            0, s, c, 0,
            0, 0, 0, 1});
        // clang-format on
        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_y(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        // clang-format off
        Matrix4x4 res({
            c, 0, -s, 0,
            0, 1, 0, 0,
            s, 0, c, 0,
            0, 0, 0, 1});
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_z(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);
        // clang-format off
        Matrix4x4 res({
            c, -s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1});
        // clang-format on
        return res;
    }
} // namespace di_renderer::math
