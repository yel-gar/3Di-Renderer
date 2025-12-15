#include "MatrixTransforms.hpp"

#include <cmath>

namespace di_renderer::math {
    Matrix4x4 MatrixTransforms::translate(const Vector3& offset) {
        // clang-format off
        Matrix4x4 res({
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            offset.x, offset.y, offset.z, 1
        });
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::scale(const Vector3& scale) {
        // clang-format off
        Matrix4x4 res({
            scale.x, 0, 0, 0,
            0, scale.y, 0, 0,
            0, 0, scale.z, 0,
            0, 0, 0, 1
        });
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_x(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        // clang-format off
        Matrix4x4 res({
            1, 0, 0, 0,
            0, c, s, 0,
            0, -s, c, 0,
            0, 0, 0, 1
        });
        // clang-format on
        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_y(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);

        // clang-format off
        Matrix4x4 res({
            c, 0, s, 0,
            0, 1, 0, 0,
            -s, 0, c, 0,
            0, 0, 0, 1
        });
        // clang-format on

        return res;
    }

    Matrix4x4 MatrixTransforms::rotate_z(float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);
        // clang-format off
        Matrix4x4 res({
            c, s, 0, 0,
            -s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        });
        // clang-format on
        return res;
    }

    /**
     *
     * @param eye - положение просмотра
     * @param target - вектор направления просмотра
     * @param up - вектор определяющий верх
     * @return - view matrix (V)
     */
    Matrix4x4 MatrixTransforms::look_at(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 z = (eye - target).normalized();
        Vector3 x = (up.cross(z)).normalized();
        Vector3 y = (z.cross(x)).normalized();

        // clang-format off
        // сразу перемноженная матрица P * T
        Matrix4x4 res({
            x.x, y.x, z.x, 0,
            x.y, y.y, z.y, 0,
            x.z, y.z, z.z, 0,
            -eye.dot(x), -eye.dot(y), -eye.dot(z), 1
        });
        // clang-format on
        return res;
    }

    /**
     *
     * @param fov_radians - fov в радианах (полный fov)
     * @param aspect_ratio - соотношение сторон
     * @param near_plane - ближняя зона видимости
     * @param far_plane - дальняя зона видимости
     * @return - projection matrix (P) ну или перспектива
     */
    Matrix4x4 MatrixTransforms::perspective(float fov_radians, float aspect_ratio, float near_plane, float far_plane) {
        float tan_of_half_fov = std::tan(fov_radians * 0.5F);
        // clang-format off
        Matrix4x4 res({
            1 / tan_of_half_fov, 0, 0, 0,
            0, 1 / (aspect_ratio * tan_of_half_fov), 0, 0,
            0, 0, -(far_plane + near_plane) / (far_plane - near_plane), -1,
            0, 0, -2 * far_plane * near_plane / (far_plane - near_plane), 0
        });
        // clang-format on
        return res;
    }

} // namespace di_renderer::math
