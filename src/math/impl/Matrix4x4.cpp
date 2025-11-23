#include "../Matrix4x4.hpp"

#include <array>
#include <cmath>

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
                set(row, col, func(row, col));
            }
        }
    }

    Matrix4x4 Matrix4x4::identity()
    {
        Matrix4x4 result = Matrix4x4();
        for (size_t i = 0; i < 4; i++)
        {
            result.set(i, i, 1.0F);
        }
        return result;
    }

    Matrix4x4 Matrix4x4::transposed() const
    {
        Matrix4x4 result;
        for (size_t row = 0; row < 4; row++)
        {
            for (size_t col = 0; col < 4; col++)
            {
                result.set(col, row, get(row, col)); // NOLINT(readability-suspicious-call-argument)
            }
        }
        return result;
    }

    float Matrix4x4::get(const size_t row, const size_t col) const
    {
        if (row >= 4 || col >= 4)
        {
            return 0.0F;
        }
        return m_data[(row * 4) + col]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    void Matrix4x4::set(const size_t row, const size_t col, const float value)
    {
        if (row >= 4 || col >= 4)
        {
            return;
        }
        m_data[(row * 4) + col] = value; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (size_t i = 0; i < 16; i++)
        {
            result.m_data[i] = m_data[i] + other.m_data[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }
        return result;
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (size_t i = 0; i < 16; i++)
        {
            result.m_data[i] = m_data[i] - other.m_data[i]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }
        return result;
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
    {
        Matrix4x4 result;
        for (size_t row = 0; row < 4; row++)
        {
            for (size_t col = 0; col < 4; col++)
            {
                float sum = 0.0F;
                for (size_t k = 0; k < 4; k++)
                {
                    sum += get(row, k) * other.get(k, col);
                }
                result.set(row, col, sum);
            }
        }
        return result;
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
        const float x = (get(0, 0) * vec.x) + (get(0, 1) * vec.y) + (get(0, 2) * vec.z) + (get(0, 3) * vec.w);
        const float y = (get(1, 0) * vec.x) + (get(1, 1) * vec.y) + (get(1, 2) * vec.z) + (get(1, 3) * vec.w);
        const float z = (get(2, 0) * vec.x) + (get(2, 1) * vec.y) + (get(2, 2) * vec.z) + (get(2, 3) * vec.w);
        const float w = (get(3, 0) * vec.x) + (get(3, 1) * vec.y) + (get(3, 2) * vec.z) + (get(3, 3) * vec.w);

        return {x, y, z, w};
    }
} // namespace di_renderer::math
