#pragma once

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

        Vector3& operator+=(const Vector3& other);
        Vector3& operator-=(const Vector3& other);
    };
}
