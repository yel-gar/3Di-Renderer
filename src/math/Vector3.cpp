#include "Vector3.hpp"

#include <cmath>
#include <limits>

namespace di_renderer::math {

    Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

    Vector3::Vector3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

    float Vector3::length() const {
        return std::sqrt((x * x) + (y * y) + (z * z));
    }

    Vector3 Vector3::normalized() const {
        const float length = this->length();
        if (length <= EPSILON) {
            return {};
        }
        return {x / length, y / length, z / length};
    }

    float Vector3::dot(const Vector3& other) const {
        return (x * other.x) + (y * other.y) + (z * other.z);
    }

    Vector3 Vector3::cross(const Vector3& other) const {
        return {(y * other.z) - (z * other.y), (z * other.x) - (x * other.z), (x * other.y) - (y * other.x)};
    }

    Vector3 Vector3::operator+(const Vector3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vector3 Vector3::operator-(const Vector3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vector3 Vector3::operator*(float value) const {
        return {x * value, y * value, z * value};
    }
    Vector3& Vector3::operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3& Vector3::operator-=(const Vector3& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3& Vector3::operator*=(float value) {
        x *= value;
        y *= value;
        z *= value;
        return *this;
    }

    bool Vector3::operator==(const Vector3& other) const {
        return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON && std::abs(z - other.z) < EPSILON;
    }
} // namespace di_renderer::math
