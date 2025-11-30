#include "core/Transform.hpp"
#include "math/MatrixTransforms.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace di_renderer::math;
using namespace di_renderer::core;

TEST(TransformTests, Translation) {
    Vector3 offset(10, -5, 3);
    Matrix4x4 t = MatrixTransforms::translate(offset);

    Vector4 point(0, 0, 0, 1);
    Vector4 result = t * point;

    EXPECT_EQ(result, Vector4(10, -5, 3, 1));
}

TEST(TransformTests, Scaling) {
    Vector3 scale(2, 0.5, 3.0);
    Matrix4x4 s = MatrixTransforms::scale(scale);

    Vector4 point(10, 10, 10, 1);
    Vector4 result = s * point;

    EXPECT_EQ(result, Vector4(20, 5, 30, 1));
}

TEST(TransformTests, RotationZ) {
    float angle = M_PI / 2;
    Matrix4x4 r = MatrixTransforms::rotate_z(angle);

    Vector4 vec_x(1, 0, 0, 1);
    Vector4 result = r * vec_x;

    EXPECT_EQ(result, Vector4(0, 1, 0, 1));
}

TEST(TransformTests, ModelMatrixTRS) {
    Vector3 scale(2, 2, 2);
    float angle = M_PI / 2;
    Vector3 tr(0, 5, 0);

    Matrix4x4 s = MatrixTransforms::scale(scale);
    Matrix4x4 r = MatrixTransforms::rotate_z(angle);
    Matrix4x4 t = MatrixTransforms::translate(tr);

    Matrix4x4 model = t * r * s;

    // Точка (1, 0, 0)
    // 1. Scale(2) -> (2, 0, 0)
    // 2. RotZ(90) -> (0, 2, 0)
    // 3. Trans(0,5,0) -> (0, 7, 0)

    Vector4 p(1, 0, 0, 1);
    Vector4 result = model * p;

    EXPECT_EQ(result, Vector4(0, 7, 0, 1));
}

TEST(TransformTests, InitialState) {
    Transform t;
    EXPECT_EQ(t.get_matrix(), Matrix4x4::identity());
}

TEST(TransformTests, TranslationChangedCheck) {
    Transform t;
    t.set_position(Vector3(10, 0, 0));

    const Matrix4x4& m = t.get_matrix();
    EXPECT_FLOAT_EQ(m(0, 3), 10.0F);

    t.translate(Vector3(5, 0, 0));
    EXPECT_FLOAT_EQ(t.get_matrix()(0, 3), 15.0F);
}

TEST(TransformTests, FullTransformation) {
    Transform t;
    t.set_position(Vector3(1, 2, 3));
    t.set_rotation(Vector3(M_PI / 2.0F, 0, 0));
    t.set_scale(Vector3(2, 2, 2));

    // Точка (0, 1, 0)
    // 1. Scale -> (0, 2, 0)
    // 2. RotateX(90) -> (0, 0, 2) (y переходит в z)
    // 3. Translate -> (1, 2, 5)

    Vector4 local_point(0, 1, 0, 1);
    Vector4 world_point = t.get_matrix() * local_point;

    EXPECT_EQ(world_point, Vector4(1, 2, 5, 1));
}
