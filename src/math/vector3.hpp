#pragma once

namespace Math {
    class Vector3 {
    public:
        float x, y, z;

        Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f);

        // Basic operations
        float length() const;
        Vector3 normalize() const;
        float dot(const Vector3& other) const;
        Vector3 cross(const Vector3& other) const;

        // Operators
        Vector3 operator+(const Vector3& other) const;
        Vector3 operator-(const Vector3& other) const;
        Vector3 operator*(float scalar) const;
        Vector3 operator/(float scalar) const;

        // Compound assignment
        Vector3& operator+=(const Vector3& other);
        Vector3& operator-=(const Vector3& other);
        Vector3& operator*=(float scalar);

        // Static constants
        static const Vector3 ZERO;
        static const Vector3 UP;
        static const Vector3 RIGHT;
        static const Vector3 FORWARD;
    };
}