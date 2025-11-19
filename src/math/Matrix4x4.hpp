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

        Matrix4x4(const std::array<float, 16>& values);

        Matrix4x4(const std::function<float(int, int)>& func);

        static Matrix4x4 identity();
        Matrix4x4 transposed() const;

        float get(int row, int col) const;
        void set(int row, int col, float value);

        Matrix4x4 operator+(const Matrix4x4& other) const;
        Matrix4x4 operator-(const Matrix4x4& other) const;
        Matrix4x4 operator*(const Matrix4x4& other) const;

        Matrix4x4& operator+=(const Matrix4x4& other);
        Matrix4x4& operator-=(const Matrix4x4& other);
        Matrix4x4& operator*=(const Matrix4x4& other);

        Vector4 operator*(const Vector4& vec) const;

      private:
        std::array<float, 16> m_;
    };
} // namespace di_renderer::math
