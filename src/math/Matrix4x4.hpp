#pragma once
#include "Vector4.hpp"

#include <array>
#include <functional>

namespace di_renderer::math
{
    class Matrix4x4
    {
      public:
        Matrix4x4();

        explicit Matrix4x4(const std::array<float, 16>& values);

        explicit Matrix4x4(const std::function<float(int, int)>& func);

        static Matrix4x4 identity();
        Matrix4x4 transposed() const;

        float get(size_t row, size_t col) const;
        void set(size_t row, size_t col, float value);

        Matrix4x4 operator+(const Matrix4x4& other) const;
        Matrix4x4 operator-(const Matrix4x4& other) const;
        Matrix4x4 operator*(const Matrix4x4& other) const;

        Matrix4x4& operator+=(const Matrix4x4& other);
        Matrix4x4& operator-=(const Matrix4x4& other);
        Matrix4x4& operator*=(const Matrix4x4& other);

        Vector4 operator*(const Vector4& vec) const;

      private:
        std::array<float, 16> m_data{};
    };
} // namespace di_renderer::math
