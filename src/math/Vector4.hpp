#pragma once

#include <limits>
namespace di_renderer::math {
    class Vector4 {
      public:
        float x;
        float y;
        float z;
        float w;

        Vector4();
        Vector4(float x, float y, float z, float w);

        float length() const;
        Vector4 normalized() const;

        float dot(const Vector4& other) const;

        Vector4 operator+(const Vector4& other) const;
        Vector4 operator-(const Vector4& other) const;
        Vector4 operator*(float value) const;
        Vector4 operator/(float value) const;

        Vector4& operator+=(const Vector4& other);
        Vector4& operator-=(const Vector4& other);

        bool operator==(const Vector4& other) const;

      private:
        constexpr const static float EPSILON = std::numeric_limits<float>::epsilon();
    };

    inline Vector4 operator*(float value, const Vector4& vec) {
        return vec * value;
    }

    inline Vector4 operator/(float value, const Vector4& vec) {
        return vec / value;
    }
} // namespace di_renderer::math
