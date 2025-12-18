#include "math/Matrix4x4.hpp"
#include "math/MatrixTransforms.hpp"
#include "math/Transform.hpp"
#include "math/UVCoord.hpp"
#include "math/Vector3.hpp"
#include "math/Vector4.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace di_renderer::math;

TEST(UVCoordTests, Construction) {
    const UVCoord def;
    EXPECT_FLOAT_EQ(def.u, 0.0f);
    EXPECT_FLOAT_EQ(def.v, 0.0f);

    const UVCoord param(0.5f, 1.0f);
    EXPECT_FLOAT_EQ(param.u, 0.5f);
    EXPECT_FLOAT_EQ(param.v, 1.0f);
}

TEST(Vector3Tests, ConstructionAndNormallization) {
    const Vector3 vec(3.0f, 0.0f, 0.0f);
    EXPECT_FLOAT_EQ(vec.x, 3.0f);

    const Vector3 norm = vec.normalized();
    const Vector3 expected_norm(1.0f, 0.0f, 0.0f);
    EXPECT_EQ(norm, expected_norm);
    EXPECT_FLOAT_EQ(norm.length(), 1.0f);

    const Vector3 vector(321, 2.718281828, 3.1415);
    EXPECT_FLOAT_EQ(vector.normalized().length(), 1.0f);
    const Vector3 zero;
    const Vector3 zero_norm = zero.normalized();
    EXPECT_FLOAT_EQ(zero_norm.length(), 0.0f);
}

TEST(Vector3Tests, Arithmetic) {
    Vector3 v1(1.0f, 2.0f, 3.0f);
    const Vector3 v2(4.0f, 5.0f, 6.0f);

    const Vector3 sum = v1 + v2;
    const Vector3 expected_sum(5.0f, 7.0f, 9.0f);
    EXPECT_EQ(sum, expected_sum);

    const Vector3 diff = v2 - v1;
    const Vector3 expected_diff(3.0f, 3.0f, 3.0f);
    EXPECT_EQ(diff, expected_diff);

    v1 += Vector3(1.0f, 1.0f, 1.0f);
    const Vector3 expected_v1(2.0f, 3.0f, 4.0f);
    EXPECT_EQ(v1, expected_v1);
}

TEST(Vector3Tests, Products) {
    const Vector3 right(1.0f, 0.0f, 0.0f);
    const Vector3 up(0.0f, 1.0f, 0.0f);

    EXPECT_FLOAT_EQ(right.dot(up), 0.0f);

    const Vector3 v(2.0f, 2.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.dot(right), 2.0f);

    const Vector3 forward = right.cross(up);
    const Vector3 expected_cross = Vector3(0.0f, 0.0f, 1.0f);
    EXPECT_EQ(forward, expected_cross);

    const Vector3 backward = up.cross(right);
    EXPECT_FLOAT_EQ(backward.z, -1.0f);
}

TEST(Vector3Tests, Length) {
    const Vector3 v(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(v.length(), 5.0f);

    const Vector3 zero;
    EXPECT_FLOAT_EQ(zero.length(), 0.0f);
}

TEST(Vector4Tests, Arithmetic) {
    Vector4 v1(10, 20, 30, 40);
    const Vector4 v2(1, 2, 3, 4);

    const Vector4 sum = v1 + v2;
    EXPECT_EQ(sum, Vector4(11, 22, 33, 44));

    const Vector4 diff = v1 - v2;
    EXPECT_EQ(diff, Vector4(9, 18, 27, 36));

    v1 += v2;
    EXPECT_EQ(v1, Vector4(11, 22, 33, 44));
}

TEST(Vector4Tests, DotAndLength) {
    const Vector4 v1(1, 0, 0, 0);
    const Vector4 v2(0, 1, 0, 0);
    EXPECT_FLOAT_EQ(v1.dot(v2), 0.0f);

    const Vector4 v3(2, 2, 2, 2);
    EXPECT_FLOAT_EQ(v3.dot(v3), 16.0f);
    EXPECT_FLOAT_EQ(v3.length(), 4.0f);
    EXPECT_EQ(v3.normalized(), Vector4(0.5f, 0.5f, 0.5f, 0.5f));
}

TEST(Matrix4x4Tests, ConstructorArray) {
    const std::array<float, 16> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    Matrix4x4 m(data);

    EXPECT_FLOAT_EQ(m(0, 0), 1.0f);
    EXPECT_FLOAT_EQ(m(3, 0), 4.0f);
    EXPECT_FLOAT_EQ(m(0, 3), 13.0f);
    EXPECT_FLOAT_EQ(m(3, 3), 16.0f);
}

TEST(Matrix4x4Tests, Identity) {
    const Matrix4x4 id = Matrix4x4::identity();
    const Matrix4x4 expected_identity = Matrix4x4({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1});
    EXPECT_EQ(id, expected_identity);
}

TEST(Matrix4x4Tests, Transposed) {
    Matrix4x4 m = Matrix4x4::identity();
    m(0, 3) = 5.0f;
    m(3, 0) = 2.0f;

    const Matrix4x4 t = m.transposed();

    Matrix4x4 expected = Matrix4x4::identity();
    expected(3, 0) = 5.0f;
    expected(0, 3) = 2.0f;

    EXPECT_EQ(t, expected);
}

TEST(Matrix4x4Tests, Determinant) {
    const Matrix4x4 zero;
    EXPECT_FLOAT_EQ(zero.determinant(), 0.0f);

    const Matrix4x4 id = Matrix4x4::identity();
    EXPECT_FLOAT_EQ(id.determinant(), 1.0f);

    Matrix4x4 scale = Matrix4x4::identity();
    scale(0, 0) = 2.0f;
    scale(1, 1) = 2.0f;
    scale(2, 2) = 2.0f;
    scale(3, 3) = 1.0f;

    EXPECT_FLOAT_EQ(scale.determinant(), 8.0f);
}

TEST(Matrix4x4Tests, Inverse) {
    const Matrix4x4 id = Matrix4x4::identity();
    EXPECT_EQ(id.inverse(), id);

    Matrix4x4 scale = Matrix4x4::identity();
    scale(0, 0) = 2.0f;
    scale(1, 1) = 2.0f;
    scale(2, 2) = 2.0f;
    scale(3, 3) = 1.0f;

    const Matrix4x4 inv_scale = scale.inverse();

    Matrix4x4 expected = Matrix4x4::identity();
    expected(0, 0) = 0.5f;
    expected(1, 1) = 0.5f;
    expected(2, 2) = 0.5f;
    expected(3, 3) = 1.0f;

    EXPECT_EQ(inv_scale, expected);

    Matrix4x4 translation = Matrix4x4::identity();
    translation(0, 3) = 10.0f;
    translation(1, 3) = 20.0f;
    translation(2, 3) = 30.0f;

    const Matrix4x4 res = translation * translation.inverse();
    EXPECT_EQ(res, Matrix4x4::identity());
}

TEST(Matrix4x4Tests, ChainMultiplication) {
    Matrix4x4 t = Matrix4x4::identity(); // Translate (1, 2, 3)
    t(0, 3) = 1.0f;
    t(1, 3) = 2.0f;
    t(2, 3) = 3.0f;

    Matrix4x4 s = Matrix4x4::identity(); // Scale (2, 2, 2)
    s(0, 0) = 2.0f;
    s(1, 1) = 2.0f;
    s(2, 2) = 2.0f;

    // T * S
    const Matrix4x4 model = t * s;
    const Vector4 p(1, 1, 1, 1);

    const Vector4 result = model * p;
    const Vector4 expected_vec(3.0f, 4.0f, 5.0f, 1.0f);

    EXPECT_EQ(result, expected_vec);

    const Matrix4x4 m3 = t * (s * Matrix4x4::identity());
    EXPECT_EQ(m3, model);
    t *= s;
    EXPECT_EQ(model, t);

    const Matrix4x4 t2 = Matrix4x4::identity(); // Translate (1, 2, 3)
    t(0, 3) = 1.0f;
    t(1, 3) = 2.0f;
    t(2, 3) = 3.0f;
    s *= t2;
    EXPECT_FALSE(t == s);
}

// Transform TESTS
TEST(TransformTests, Translation) {
    const Vector3 offset(10, -5, 3);
    const Matrix4x4 t = MatrixTransforms::translate(offset);

    const Vector4 point(0, 0, 0, 1);
    const Vector4 result = t * point;

    EXPECT_EQ(result, Vector4(10, -5, 3, 1));
}

TEST(TransformTests, Scaling) {
    const Vector3 scale(2, 0.5, 3.0);
    const Matrix4x4 s = MatrixTransforms::scale(scale);

    const Vector4 point(10, 10, 10, 1);
    const Vector4 result = s * point;

    EXPECT_EQ(result, Vector4(20, 5, 30, 1));
}

TEST(TransformTests, RotationZ) {
    const float angle = M_PI / 2;
    const Matrix4x4 r = MatrixTransforms::rotate_z(angle);

    const Vector4 vec_x(1, 0, 0, 1);
    const Vector4 result = r * vec_x;

    EXPECT_EQ(result, Vector4(0, 1, 0, 1));
}

TEST(TransformTests, ModelMatrixTRS) {
    const Vector3 scale(2, 2, 2);
    const float angle = M_PI / 2;
    const Vector3 tr(0, 5, 0);

    const Matrix4x4 s = MatrixTransforms::scale(scale);
    const Matrix4x4 r = MatrixTransforms::rotate_z(angle);
    const Matrix4x4 t = MatrixTransforms::translate(tr);

    const Matrix4x4 model = t * r * s;

    // Точка (1, 0, 0)
    // 1. Scale(2) -> (2, 0, 0)
    // 2. RotZ(90) -> (0, 2, 0)
    // 3. Trans(0,5,0) -> (0, 7, 0)

    const Vector4 p(1, 0, 0, 1);
    const Vector4 result = model * p;

    EXPECT_EQ(result, Vector4(0, 7, 0, 1));
}

TEST(TransformTests, InitialState) {
    const Transform t;
    EXPECT_EQ(t.get_matrix(), Matrix4x4::identity());
}

TEST(TransformTests, TranslationChangedCheck) {
    Transform t;
    t.set_position(Vector3(10, 0, 0));

    const Matrix4x4& m = t.get_matrix();
    EXPECT_FLOAT_EQ(m(0, 3), 10.0f);

    t.translate(Vector3(5, 0, 0));
    EXPECT_FLOAT_EQ(t.get_matrix()(0, 3), 15.0f);
}

TEST(TransformTests, FullTransformation) {
    Transform t;
    t.set_position(Vector3(1, 2, 3));
    t.set_rotation(Vector3(M_PI / 2.0f, 0, 0));
    t.set_scale(Vector3(2, 2, 2));

    // Точка (0, 1, 0)
    // 1. Scale -> (0, 2, 0)
    // 2. RotateX(90) -> (0, 0, 2) (y переходит в z)
    // 3. Translate -> (1, 2, 5)

    const Vector4 local_point(0, 1, 0, 1);
    const Vector4 world_point = t.get_matrix() * local_point;

    EXPECT_EQ(world_point, Vector4(1, 2, 5, 1));
}
