#include "Matrix4x4.hpp"

#include <array>
#include <cmath>
#include <stdexcept>

namespace di_renderer::math
{
    Matrix4x4::Matrix4x4()
    {
        m_data.fill(0.0F);
    }

    Matrix4x4::Matrix4x4(const std::array<float, 16>& values) : m_data(values) {}

    Matrix4x4::Matrix4x4(const std::function<float(int, int)>& func)
    {
        for (int row = 0; row < 4; row++)
        {
            for (int col = 0; col < 4; col++)
            {
                (*this)(row, col) = func(row, col);
            }
        }
    }

    Matrix4x4 Matrix4x4::identity()
    {
        return Matrix4x4([](int r, int c) { return r == c ? 1.0F : 0.0F; });
    }

    Matrix4x4 Matrix4x4::transposed() const
    {
        return Matrix4x4([this](int r, int c) { return (*this)(c, r); });
    }

    float Matrix4x4::operator()(const size_t row, const size_t col) const
    {
        if (row >= 4 || col >= 4)
        {
            throw std::out_of_range("Couldn't get matrix element,parameters must be 0 < {par, par} < 4");
        }
        return m_data[(row * 4) + col]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    float& Matrix4x4::operator()(const size_t row, const size_t col)
    {
        if (row >= 4 || col >= 4)
        {
            throw std::out_of_range("Couldn't set matrix element,parameters must be 0 < (par, par) < 4");
        }
        return m_data[(row * 4) + col]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
    {
        return Matrix4x4([this, &other](int r, int c) { return (*this)(r, c) + other(r, c); });
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
    {
        return Matrix4x4([this, &other](int r, int c) { return (*this)(r, c) - other(r, c); });
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
    {
        return Matrix4x4(
            [this, &other](int r, int c)
            {
                float result = 0;
                for (int k = 0; k < 4; k++)
                {
                    result += (*this)(r, k) * other(k, c);
                }
                return result;
            });
    }

    Vector4 Matrix4x4::operator*(const Vector4& vec) const
    {
        return {((*this)(0, 0) * vec.x) + ((*this)(0, 1) * vec.y) + ((*this)(0, 2) * vec.z) + ((*this)(0, 3) * vec.w),
                ((*this)(1, 0) * vec.x) + ((*this)(1, 1) * vec.y) + ((*this)(1, 2) * vec.z) + ((*this)(1, 3) * vec.w),
                ((*this)(2, 0) * vec.x) + ((*this)(2, 1) * vec.y) + ((*this)(2, 2) * vec.z) + ((*this)(2, 3) * vec.w),
                ((*this)(3, 0) * vec.x) + ((*this)(3, 1) * vec.y) + ((*this)(3, 2) * vec.z) + ((*this)(3, 3) * vec.w)};
    }

    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other)
    {
        *this = *this + other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other)
    {
        *this = *this - other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other)
    {
        *this = *this * other;
        return *this;
    }

    bool Matrix4x4::operator==(const Matrix4x4& other) const
    {
        constexpr float eps = 1e-8F;
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                if (std::abs((*this)(i, j) - other(i, j)) > eps)
                {
                    return false; // NOLINT
                }
            }
        }
        return true;
    }

    float Matrix4x4::determinant() const
    {
        const float m00 = (*this)(0, 0);
        const float m01 = (*this)(0, 1);
        const float m02 = (*this)(0, 2);
        const float m03 = (*this)(0, 3);
        const float m10 = (*this)(1, 0);
        const float m11 = (*this)(1, 1);
        const float m12 = (*this)(1, 2);
        const float m13 = (*this)(1, 3);
        const float m20 = (*this)(2, 0);
        const float m21 = (*this)(2, 1);
        const float m22 = (*this)(2, 2);
        const float m23 = (*this)(2, 3);
        const float m30 = (*this)(3, 0);
        const float m31 = (*this)(3, 1);
        const float m32 = (*this)(3, 2);
        const float m33 = (*this)(3, 3);

        float min0 = (m00 * m11) - (m01 * m10);
        float min1 = (m00 * m12) - (m02 * m10);
        float min2 = (m00 * m13) - (m03 * m10);
        float min3 = (m01 * m12) - (m02 * m11);
        float min4 = (m01 * m13) - (m03 * m11);
        float min5 = (m02 * m13) - (m03 * m12);
        float cof5 = (m22 * m33) - (m23 * m32);
        float cof4 = (m21 * m33) - (m23 * m31);
        float cof3 = (m21 * m32) - (m22 * m31);
        float cof2 = (m20 * m33) - (m23 * m30);
        float cof1 = (m20 * m32) - (m22 * m30);
        float cof0 = (m20 * m31) - (m21 * m30);

        return ((min0 * cof5) - (min1 * cof4) + (min2 * cof3) + (min3 * cof2) - (min4 * cof1) + (min5 * cof0));
    }

    Matrix4x4 Matrix4x4::inverse() const
    {
        Matrix4x4 inv;

        const float m00 = (*this)(0, 0);
        const float m01 = (*this)(0, 1);
        const float m02 = (*this)(0, 2);
        const float m03 = (*this)(0, 3);
        const float m10 = (*this)(1, 0);
        const float m11 = (*this)(1, 1);
        const float m12 = (*this)(1, 2);
        const float m13 = (*this)(1, 3);
        const float m20 = (*this)(2, 0);
        const float m21 = (*this)(2, 1);
        const float m22 = (*this)(2, 2);
        const float m23 = (*this)(2, 3);
        const float m30 = (*this)(3, 0);
        const float m31 = (*this)(3, 1);
        const float m32 = (*this)(3, 2);
        const float m33 = (*this)(3, 3);

        float a2323 = (m22 * m33) - (m23 * m32);
        float a1323 = (m21 * m33) - (m23 * m31);
        float a1223 = (m21 * m32) - (m22 * m31);
        float a0323 = (m20 * m33) - (m23 * m30);
        float a0223 = (m20 * m32) - (m22 * m30);
        float a0123 = (m20 * m31) - (m21 * m30);

        inv(0, 0) = ((m11 * a2323) - (m12 * a1323) + (m13 * a1223));
        inv(1, 0) = -((m10 * a2323) - (m12 * a0323) + (m13 * a0223));
        inv(2, 0) = ((m10 * a1323) - (m11 * a0323) + (m13 * a0123));
        inv(3, 0) = -((m10 * a1223) - (m11 * a0223) + (m12 * a0123));

        float det = (m00 * inv(0, 0)) + (m01 * inv(1, 0)) + (m02 * inv(2, 0)) + (m03 * inv(3, 0));

        if (std::abs(det) < 1e-5F)
        {
            return identity();
        }

        float inv_det = 1.0F / det;

        float a2313 = (m12 * m33) - (m13 * m32);
        float a1313 = (m11 * m33) - (m13 * m31);
        float a1213 = (m11 * m32) - (m12 * m31);
        float a2312 = (m12 * m23) - (m13 * m22);
        float a1312 = (m11 * m23) - (m13 * m21);
        float a1212 = (m11 * m22) - (m12 * m21);

        inv(0, 1) = -((m01 * a2323) - (m02 * a1323) + (m03 * a1223));
        inv(1, 1) = ((m00 * a2323) - (m02 * a0323) + (m03 * a0223));
        inv(2, 1) = -((m00 * a1323) - (m01 * a0323) + (m03 * a0123));
        inv(3, 1) = ((m00 * a1223) - (m01 * a0223) + (m02 * a0123));

        float a0313 = (m10 * m33) - (m13 * m30);
        float a0213 = (m10 * m32) - (m12 * m30);
        float a0113 = (m10 * m31) - (m11 * m30);

        inv(0, 2) = ((m01 * a2313) - (m02 * a1313) + (m03 * a1213));
        inv(1, 2) = -((m00 * a2313) - (m02 * a0313) + (m03 * a0213));
        inv(2, 2) = ((m00 * a1313) - (m01 * a0313) + (m03 * a0113));
        inv(3, 2) = -((m00 * a1213) - (m01 * a0213) + (m02 * a0113));

        float a0312 = (m10 * m23) - (m13 * m20);
        float a0212 = (m10 * m22) - (m12 * m20);
        float a0112 = (m10 * m21) - (m11 * m20);

        inv(0, 3) = -((m01 * a2312) - (m02 * a1312) + (m03 * a1212));
        inv(1, 3) = ((m00 * a2312) - (m02 * a0312) + (m03 * a0212));
        inv(2, 3) = -((m00 * a1312) - (m01 * a0312) + (m03 * a0112));
        inv(3, 3) = ((m00 * a1212) - (m01 * a0212) + (m02 * a0112));

        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                inv(r, c) *= inv_det;
            }
        }

        return inv;
    }
} // namespace di_renderer::math
