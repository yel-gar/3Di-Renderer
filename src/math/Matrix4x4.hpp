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

        float determinant() const;
        Matrix4x4 inverse() const;

        float& operator()(size_t row, size_t col);
        float operator()(size_t row, size_t col) const;

        Matrix4x4 operator+(const Matrix4x4& other) const;
        Matrix4x4 operator-(const Matrix4x4& other) const;
        Matrix4x4 operator*(const Matrix4x4& other) const;

        Matrix4x4& operator+=(const Matrix4x4& other);
        Matrix4x4& operator-=(const Matrix4x4& other);
        Matrix4x4& operator*=(const Matrix4x4& other);

        Vector4 operator*(const Vector4& vec) const;

        bool operator==(const Matrix4x4& other) const;

      private:
        static constexpr float EPS = 1e-8F;
        std::array<float, 16> m_data{};
    };
} // namespace di_renderer::math
