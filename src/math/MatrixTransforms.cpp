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

    Matrix4x4 MatrixTransforms::rotate_x(const float angle) {
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

    Matrix4x4 MatrixTransforms::rotate_y(const float angle) {
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

    Matrix4x4 MatrixTransforms::rotate_z(const float angle) {
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
        const Vector3 forward = (target - eye).normalized(); // Direction FROM eye TO target
        const Vector3 right = forward.cross(up).normalized();
        const Vector3 new_up = right.cross(forward).normalized();

        // clang-format off
        Matrix4x4 const rotation({
            right.x, right.y, right.z, 0,
            new_up.x, new_up.y, new_up.z, 0,
            -forward.x, -forward.y, -forward.z, 0,  // Negative forward for right-handed system
            0, 0, 0, 1
        });

        Matrix4x4 const translation({
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            -eye.x, -eye.y, -eye.z, 1
        });

        return rotation * translation;
        // clang-format on
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
        const float tan_of_half_fov = std::tan(fov_radians * 0.5f);
        const float f = 1.0f / tan_of_half_fov;

        // clang-format off
    Matrix4x4 res({
        f / aspect_ratio, 0, 0, 0,
        0, f, 0, 0,
        0, 0, -(far_plane + near_plane) / (far_plane - near_plane), -1,
        0, 0, -2 * far_plane * near_plane / (far_plane - near_plane), 0
    });
        // clang-format on
        return res;
    }

} // namespace di_renderer::math
