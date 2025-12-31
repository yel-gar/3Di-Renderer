#include "render/Camera.hpp"

#include <cmath>
#include <gtest/gtest.h>
using namespace di_renderer::render;
using namespace di_renderer::math;

TEST(CameraTests, ViewMatrix_Identity) {
    const Camera cam({0, 0, 0}, {0, 0, -1}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 const expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ViewMatrix_TranslationZ) {
    const Camera cam({0, 0, 10}, {0, 0, 0}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 const expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, -10.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ViewMatrix_TranslationX_Strafing) {
    Camera cam({0, 0, 0}, {0, 0, -1}, 1.0F, 1.0F, 0.1F, 100.0F);
    cam.move({5.0F, 0.0F, 0.0F});
    // clang-format off
    Matrix4x4 const expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        -5.0F, 0.0F, 0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ViewMatrix_Rotation_90Degrees) {
    // Z (Forward) = (1, 0, 0)
    // X (Right)   = Up x Z = (0, 0, -1)
    // Y (New Up)  = Z x X  = (0, 1, 0)
    const Camera cam({0, 0, 0}, {-1, 0, 0}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 const expected({
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 1.0F,  0.0F, 0.0F,
        -1.0F, 0.0F,  0.0F, 0.0F,
        0.0F, 0.0F,  0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ProjectionMatrix_Calculated) {
    const float fov = 3.1415926535F / 2.0F;
    const float aspect = 2.0F;
    const float n = 1.0F;
    const float f = 3.0F;

    const Camera cam({0, 0, 0}, {0, 0, 1}, fov, aspect, n, f);

    // tan(45) = 1.
    // [0,0] = 1/tan = 1
    // [1,1] = 1/(2*1) = 0.5
    // [2,2] = -(3+1)/(3-1) = -2
    // [2,3] = -(2*3*1/2) = -3
    // [3,1] = -1
    // clang-format off
    Matrix4x4 const expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 0.5F, 0.0F, 0.0F,
        0.0F, 0.0F, -2.0F, -1.0F,
        0.0F, 0.0F, -3.0F, 0.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_projection_matrix(), expected);
}

TEST(CameraTests, Setters_UpdateMatrices) {
    Camera cam({0, 0, 0}, {0, 0, -1}, 1.0F, 1.0F, 0.1F, 100.0F);

    cam.move({0, 10, 0});
    // clang-format off
    Matrix4x4 const expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, -10.0F, 0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}
