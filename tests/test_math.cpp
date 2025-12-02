#include "math/Matrix4x4.hpp"
#include "math/MatrixTransforms.hpp"
#include "math/Transform.hpp"
#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace di_renderer::math;
using namespace di_renderer::core;

TEST(UVCoordTests, Construction) {
    const UVCoord def;
    EXPECT_FLOAT_EQ(def.u, 0.0F);
    EXPECT_FLOAT_EQ(def.v, 0.0F);

    const UVCoord param(0.5F, 1.0F);
    EXPECT_FLOAT_EQ(param.u, 0.5F);
    EXPECT_FLOAT_EQ(param.v, 1.0F);
}

TEST(Vector3Tests, ConstructionAndNormallization) {
    const Vector3 vec(3.0F, 0.0F, 0.0F);
    EXPECT_FLOAT_EQ(vec.x, 3.0F);

    const Vector3 norm = vec.normalized();
    const Vector3 expected_norm(1.0F, 0.0F, 0.0F);
    EXPECT_EQ(norm, expected_norm);
    EXPECT_FLOAT_EQ(norm.length(), 1.0F);

    const Vector3 vector(321, 2.718281828, 3.1415);
    EXPECT_FLOAT_EQ(vector.normalized().length(), 1.0F);
    const Vector3 zero;
    const Vector3 zero_norm = zero.normalized();
    EXPECT_FLOAT_EQ(zero_norm.length(), 0.0F);
}

TEST(Vector3Tests, Arithmetic) {
    Vector3 v1(1.0F, 2.0F, 3.0F);
    Vector3 v2(4.0F, 5.0F, 6.0F);

    const Vector3 sum = v1 + v2;
    const Vector3 expected_sum(5.0F, 7.0F, 9.0F);
    EXPECT_EQ(sum, expected_sum);

    const Vector3 diff = v2 - v1;
    const Vector3 expected_diff(3.0F, 3.0F, 3.0F);
    EXPECT_EQ(diff, expected_diff);

    v1 += Vector3(1.0F, 1.0F, 1.0F);
    const Vector3 expected_v1(2.0F, 3.0F, 4.0F);
    EXPECT_EQ(v1, expected_v1);
}

TEST(Vector3Tests, Products) {
    Vector3 right(1.0F, 0.0F, 0.0F);
    Vector3 up(0.0F, 1.0F, 0.0F);

    EXPECT_FLOAT_EQ(right.dot(up), 0.0F);

    Vector3 v(2.0F, 2.0F, 0.0F);
    EXPECT_FLOAT_EQ(v.dot(right), 2.0F);

    Vector3 forward = right.cross(up);
    Vector3 expected_cross = Vector3(0.0F, 0.0F, 1.0F);
    EXPECT_EQ(forward, expected_cross);

    Vector3 backward = up.cross(right);
    EXPECT_FLOAT_EQ(backward.z, -1.0F);
}

TEST(Vector3Tests, Length) {
    Vector3 v(3.0F, 4.0F, 0.0F);
    EXPECT_FLOAT_EQ(v.length(), 5.0F);

    Vector3 zero;
    EXPECT_FLOAT_EQ(zero.length(), 0.0F);
}

TEST(Vector4Tests, Arithmetic) {
    Vector4 v1(10, 20, 30, 40);
    Vector4 v2(1, 2, 3, 4);

    Vector4 sum = v1 + v2;
    EXPECT_EQ(sum, Vector4(11, 22, 33, 44));

    Vector4 diff = v1 - v2;
    EXPECT_EQ(diff, Vector4(9, 18, 27, 36));

    v1 += v2;
    EXPECT_EQ(v1, Vector4(11, 22, 33, 44));
}

TEST(Vector4Tests, DotAndLength) {
    Vector4 v1(1, 0, 0, 0);
    Vector4 v2(0, 1, 0, 0);
    EXPECT_FLOAT_EQ(v1.dot(v2), 0.0F);

    Vector4 v3(2, 2, 2, 2);
    EXPECT_FLOAT_EQ(v3.dot(v3), 16.0F);
    EXPECT_FLOAT_EQ(v3.length(), 4.0F);
    EXPECT_EQ(v3.normalized(), Vector4(0.5F, 0.5F, 0.5F, 0.5F));
}

TEST(Matrix4x4Tests, ConstructorArray) {
    std::array<float, 16> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    Matrix4x4 m(data);

    EXPECT_FLOAT_EQ(m(0, 0), 1.0F);
    EXPECT_FLOAT_EQ(m(0, 3), 4.0F);
    EXPECT_FLOAT_EQ(m(3, 3), 16.0F);
}

TEST(Matrix4x4Tests, Identity) {
    Matrix4x4 id = Matrix4x4::identity();
    Matrix4x4 expected_identity = Matrix4x4({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
    EXPECT_EQ(id, expected_identity);
}

TEST(Matrix4x4Tests, Transposed) {
    Matrix4x4 m = Matrix4x4::identity();
    m(0, 3) = 5.0F;
    m(3, 0) = 2.0F;

    Matrix4x4 t = m.transposed();

    Matrix4x4 expected = Matrix4x4::identity();
    expected(3, 0) = 5.0F;
    expected(0, 3) = 2.0F;

    EXPECT_EQ(t, expected);
}

TEST(Matrix4x4Tests, Determinant) {
    Matrix4x4 zero;
    EXPECT_FLOAT_EQ(zero.determinant(), 0.0F);

    Matrix4x4 id = Matrix4x4::identity();
    EXPECT_FLOAT_EQ(id.determinant(), 1.0F);

    Matrix4x4 scale = Matrix4x4::identity();
    scale(0, 0) = 2.0F;
    scale(1, 1) = 2.0F;
    scale(2, 2) = 2.0F;
    scale(3, 3) = 1.0F;

    EXPECT_FLOAT_EQ(scale.determinant(), 8.0F);
}

TEST(Matrix4x4Tests, Inverse) {
    Matrix4x4 id = Matrix4x4::identity();
    EXPECT_EQ(id.inverse(), id);

    Matrix4x4 scale = Matrix4x4::identity();
    scale(0, 0) = 2.0F;
    scale(1, 1) = 2.0F;
    scale(2, 2) = 2.0F;
    scale(3, 3) = 1.0F;

    Matrix4x4 inv_scale = scale.inverse();

    Matrix4x4 expected = Matrix4x4::identity();
    expected(0, 0) = 0.5F;
    expected(1, 1) = 0.5F;
    expected(2, 2) = 0.5F;
    expected(3, 3) = 1.0F;

    EXPECT_EQ(inv_scale, expected);

    Matrix4x4 translation = Matrix4x4::identity();
    translation(0, 3) = 10.0F;
    translation(1, 3) = 20.0F;
    translation(2, 3) = 30.0F;

    Matrix4x4 res = translation * translation.inverse();
    EXPECT_EQ(res, Matrix4x4::identity());
}

TEST(Matrix4x4Tests, ChainMultiplication) {
    Matrix4x4 t = Matrix4x4::identity(); // Translate (1, 2, 3)
    t(0, 3) = 1.0F;
    t(1, 3) = 2.0F;
    t(2, 3) = 3.0F;

    Matrix4x4 s = Matrix4x4::identity(); // Scale (2, 2, 2)
    s(0, 0) = 2.0F;
    s(1, 1) = 2.0F;
    s(2, 2) = 2.0F;

    // T * S
    Matrix4x4 model = t * s;
    Vector4 p(1, 1, 1, 1);

    Vector4 result = model * p;
    Vector4 expected_vec(3.0F, 4.0F, 5.0F, 1.0F);

    EXPECT_EQ(result, expected_vec);

    const Matrix4x4 m3 = t * (s * Matrix4x4::identity());
    EXPECT_EQ(m3, model);
    t *= s;
    EXPECT_EQ(model, t);

    Matrix4x4 t2 = Matrix4x4::identity(); // Translate (1, 2, 3)
    t(0, 3) = 1.0F;
    t(1, 3) = 2.0F;
    t(2, 3) = 3.0F;
    s *= t2;
    EXPECT_FALSE(t == s);
}

// Transform TESTS
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
