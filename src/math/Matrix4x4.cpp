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

    Vector4 Matrix4x4::operator*(const Vector4& vec) const
    {
        return {((*this)(0, 0) * vec.x) + ((*this)(0, 1) * vec.y) + ((*this)(0, 2) * vec.z) + ((*this)(0, 3) * vec.w),
                ((*this)(1, 0) * vec.x) + ((*this)(1, 1) * vec.y) + ((*this)(1, 2) * vec.z) + ((*this)(1, 3) * vec.w),
                ((*this)(2, 0) * vec.x) + ((*this)(2, 1) * vec.y) + ((*this)(2, 2) * vec.z) + ((*this)(2, 3) * vec.w),
                ((*this)(3, 0) * vec.x) + ((*this)(3, 1) * vec.y) + ((*this)(3, 2) * vec.z) + ((*this)(3, 3) * vec.w)};
    }

    float Matrix4x4::determinant() const
    {
        const float* m = m_data.data();

        float s0 = (m[0] * m[5]) - (m[1] * m[4]); // NOLINT
        float s1 = (m[0] * m[6]) - (m[2] * m[4]); // NOLINT
        float s2 = (m[0] * m[7]) - (m[3] * m[4]); // NOLINT
        float s3 = (m[1] * m[6]) - (m[2] * m[5]); // NOLINT
        float s4 = (m[1] * m[7]) - (m[3] * m[5]); // NOLINT
        float s5 = (m[2] * m[7]) - (m[3] * m[6]); // NOLINT

        float c5 = (m[10] * m[15]) - (m[11] * m[14]); // NOLINT
        float c4 = (m[9] * m[15]) - (m[11] * m[13]);  // NOLINT
        float c3 = (m[9] * m[14]) - (m[10] * m[13]);  // NOLINT
        float c2 = (m[8] * m[15]) - (m[11] * m[12]);  // NOLINT
        float c1 = (m[8] * m[14]) - (m[10] * m[12]);  // NOLINT
        float c0 = (m[8] * m[13]) - (m[9] * m[12]);   // NOLINT

        return ((s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) - (s4 * c1) + (s5 * c0));
    }

    Matrix4x4 Matrix4x4::inverse() const
    {
        const float* m = m_data.data();
        Matrix4x4 inv;
        inv.m_data.fill(0.0F);
        float* out = inv.m_data.data();

        float A2323 = m[10] * m[15] - m[11] * m[14]; // NOLINT
        float A1323 = m[9] * m[15] - m[11] * m[13];  // NOLINT
        float A1223 = m[9] * m[14] - m[10] * m[13];  // NOLINT
        float A0323 = m[8] * m[15] - m[11] * m[12];  // NOLINT
        float A0223 = m[8] * m[14] - m[10] * m[12];  // NOLINT
        float A0123 = m[8] * m[13] - m[9] * m[12];   // NOLINT

        out[0] = (m[5] * A2323 - m[6] * A1323 + m[7] * A1223);   // NOLINT
        out[4] = -(m[4] * A2323 - m[6] * A0323 + m[7] * A0223);  // NOLINT
        out[8] = (m[4] * A1323 - m[5] * A0323 + m[7] * A0123);   // NOLINT
        out[12] = -(m[4] * A1223 - m[5] * A0223 + m[6] * A0123); // NOLINT

        float det = m[0] * out[0] + m[1] * out[4] + m[2] * out[8] + m[3] * out[12]; // NOLINT

        if (std::abs(det) < 1e-5F)
        {
            return identity();
        }

        float inv_det = 1.0F / det;

        float A2313 = m[6] * m[15] - m[7] * m[14]; // NOLINT
        float A1313 = m[5] * m[15] - m[7] * m[13]; // NOLINT
        float A1213 = m[5] * m[14] - m[6] * m[13]; // NOLINT
        float A2312 = m[6] * m[11] - m[7] * m[10]; // NOLINT
        float A1312 = m[5] * m[11] - m[7] * m[9];  // NOLINT
        float A1212 = m[5] * m[10] - m[6] * m[9];  // NOLINT

        out[1] = -(m[1] * A2323 - m[2] * A1323 + m[3] * A1223); // NOLINT
        out[5] = (m[0] * A2323 - m[2] * A0323 + m[3] * A0223);  // NOLINT
        out[9] = -(m[0] * A1323 - m[1] * A0323 + m[3] * A0123); // NOLINT
        out[13] = (m[0] * A1223 - m[1] * A0223 + m[2] * A0123); // NOLINT

        float A0313 = m[4] * m[15] - m[7] * m[12]; // NOLINT
        float A0213 = m[4] * m[14] - m[6] * m[12]; // NOLINT
        float A0113 = m[4] * m[13] - m[5] * m[12]; // NOLINT

        out[2] = (m[1] * A2313 - m[2] * A1313 + m[3] * A1213);   // NOLINT
        out[6] = -(m[0] * A2313 - m[2] * A0313 + m[3] * A0213);  // NOLINT
        out[10] = (m[0] * A1313 - m[1] * A0313 + m[3] * A0113);  // NOLINT
        out[14] = -(m[0] * A1213 - m[1] * A0213 + m[2] * A0113); // NOLINT

        float A0312 = m[4] * m[11] - m[7] * m[8]; // NOLINT
        float A0212 = m[4] * m[10] - m[6] * m[8]; // NOLINT
        float A0112 = m[4] * m[9] - m[5] * m[8];  // NOLINT

        out[3] = -(m[1] * A2312 - m[2] * A1312 + m[3] * A1212);  // NOLINT
        out[7] = (m[0] * A2312 - m[2] * A0312 + m[3] * A0212);   // NOLINT
        out[11] = -(m[0] * A1312 - m[1] * A0312 + m[3] * A0112); // NOLINT
        out[15] = (m[0] * A1212 - m[1] * A0212 + m[2] * A0112);  // NOLINT

        for (int i = 0; i < 16; ++i)
        {
            out[i] *= inv_det; // NOLINT
        }

        return inv;
    }
} // namespace di_renderer::math
