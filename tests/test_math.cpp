#include "../src/math/Matrix4x4.hpp"
#include "../src/math/UVCoord.hpp"
#include "../src/math/Vector3.hpp"
#include "../src/math/Vector4.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace di_renderer::math;

TEST(UVCoordTests, Construction)
{
    const UVCoord def;
    EXPECT_FLOAT_EQ(def.u, 0.0F);
    EXPECT_FLOAT_EQ(def.v, 0.0F);

    const UVCoord param(0.5F, 1.0F);
    EXPECT_FLOAT_EQ(param.u, 0.5F);
    EXPECT_FLOAT_EQ(param.v, 1.0F);
}

TEST(Vector3Tests, ConstructionAndNormallization)
{
    const Vector3 vec(3.0F, 0.0F, 0.0F);
    EXPECT_FLOAT_EQ(vec.x, 3.0F);

    const Vector3 norm = vec.normalized();
    const Vector3 expected_norm(1.0F, 0.0F, 0.0F);
    EXPECT_TRUE(norm == expected_norm);

    const Vector3 zero;
    const Vector3 zero_norm = zero.normalized();
    EXPECT_FLOAT_EQ(zero_norm.length(), 0.0F);
}

TEST(Vector3Tests, Arithmetic)
{
    Vector3 v1(1.0F, 2.0F, 3.0F);
    Vector3 v2(4.0F, 5.0F, 6.0F);

    const Vector3 sum = v1 + v2;
    const Vector3 expected_sum(5.0F, 7.0F, 9.0F);
    EXPECT_TRUE(sum == expected_sum);

    const Vector3 diff = v2 - v1;
    const Vector3 expected_diff(3.0F, 3.0F, 3.0F);
    EXPECT_TRUE(diff == expected_diff);

    v1 += Vector3(1.0F, 1.0F, 1.0F);
    const Vector3 expected_v1(2.0F, 3.0F, 4.0F);
    EXPECT_TRUE(v1 == expected_v1);
}

TEST(Vector3Tests, Products)
{
    Vector3 right(1.0F, 0.0F, 0.0F);
    Vector3 up(0.0F, 1.0F, 0.0F);

    EXPECT_FLOAT_EQ(right.dot(up), 0.0F);

    Vector3 v(2.0F, 2.0F, 0.0F);
    EXPECT_FLOAT_EQ(v.dot(right), 2.0F);

    Vector3 forward = right.cross(up);
    Vector3 expected_cross = Vector3(0.0F, 0.0F, 1.0F);
    EXPECT_TRUE(forward == expected_cross);

    Vector3 backward = up.cross(right);
    EXPECT_FLOAT_EQ(backward.z, -1.0F);
}

TEST(Vector4Tests, BasicOperations)
{
    Vector4 v1(1.0F, 2.0F, 3.0F, 4.0F);
    Vector4 v2(1.0F, 1.0F, 1.0F, 1.0F);

    Vector4 res = v1 - v2;
    Vector4 expected_res = Vector4(0.0F, 1.0F, 2.0F, 3.0F);
    EXPECT_TRUE(res == expected_res);

    EXPECT_FLOAT_EQ(v2.length(), std::sqrt(4.0F));
}
