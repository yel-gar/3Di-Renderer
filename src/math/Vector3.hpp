#pragma once

#include <limits>
namespace di_renderer::math {
    class Vector3 {
      public:
        float x;
        float y;
        float z;

        Vector3();
        Vector3(float x, float y, float z);

        float length() const;
        Vector3 normalized() const;

        float dot(const Vector3& other) const;
        Vector3 cross(const Vector3& other) const;

        Vector3 operator+(const Vector3& other) const;
        Vector3 operator-(const Vector3& other) const;
        Vector3 operator*(float value) const;
        Vector3 operator/(float value) const;

        Vector3& operator+=(const Vector3& other);
        Vector3& operator-=(const Vector3& other);
        Vector3& operator*=(float value);

        bool operator==(const Vector3& other) const;

      private:
        constexpr const static float EPSILON = std::numeric_limits<float>::epsilon();
    };

    inline Vector3 operator*(float value, const Vector3& vec) {
        return vec * value;
    }
    inline Vector3 operator/(float value, const Vector3& vec) {
        return vec / value;
    }
} // namespace di_renderer::math
