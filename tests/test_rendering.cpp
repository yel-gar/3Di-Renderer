#include "render/Camera.hpp"

#include <cmath>
#include <gtest/gtest.h>
using namespace di_renderer::render;
using namespace di_renderer::math;

TEST(CameraTests, ViewMatrix_Identity) {
    Camera cam({0, 0, 0}, {0, 0, -1}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ViewMatrix_TranslationZ) {
    Camera cam({0, 0, 10}, {0, 0, 0}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 expected({
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
    Matrix4x4 expected({
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
    Camera cam({0, 0, 0}, {-1, 0, 0}, 1.0F, 1.0F, 0.1F, 100.0F);
    // clang-format off
    Matrix4x4 expected({
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 1.0F,  0.0F, 0.0F,
        -1.0F, 0.0F,  0.0F, 0.0F,
        0.0F, 0.0F,  0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(CameraTests, ProjectionMatrix_Calculated) {
    float fov = 3.1415926535F / 2.0F;
    float aspect = 2.0F;
    float n = 1.0F;
    float f = 3.0F;

    Camera cam({0, 0, 0}, {0, 0, 1}, fov, aspect, n, f);

    // tan(45) = 1.
    // [0,0] = 1/tan = 1
    // [1,1] = 1/(2*1) = 0.5
    // [2,2] = -(3+1)/(3-1) = -2
    // [2,3] = -(2*3*1/2) = -3
    // [3,1] = -1
    // clang-format off
    Matrix4x4 expected({
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
    Matrix4x4 expected({
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, -10.0F, 0.0F, 1.0F
    });
    // clang-format on

    EXPECT_EQ(cam.get_view_matrix(), expected);
}

TEST(MeshNormalEvaluationTest, ProvidedNormalsArePreserved) {
    std::vector<math::Vector3> vertices = {math::Vector3(0.0f, 0.0f, 0.0f), math::Vector3(1.0f, 0.0f, 0.0f),
                                           math::Vector3(0.0f, 1.0f, 0.0f)};

    std::vector<math::Vector3> provided_normals = {math::Vector3(0.0f, 0.0f, 1.0f), math::Vector3(0.0f, 0.0f, 1.0f),
                                                   math::Vector3(0.0f, 0.0f, 1.0f)};

    std::vector<std::vector<FaceVerticeData>> faces = {{{0, -1, -1}, {1, -1, -1}, {2, -1, -1}}};

    Mesh mesh(std::move(vertices), {}, std::move(provided_normals), faces);

    ASSERT_EQ(mesh.normals.size(), 3);
    EXPECT_NEAR(mesh.normals[0].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[0].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[0].z, 1.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[1].z, 1.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].x, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].y, 0.0f, 1e-5f);
    EXPECT_NEAR(mesh.normals[2].z, 1.0f, 1e-5f);
}
