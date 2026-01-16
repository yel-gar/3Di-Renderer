#include "Vector4.hpp"

#include <cmath>

namespace di_renderer::math {

    Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

    Vector4::Vector4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

    float Vector4::length() const {
        return std::sqrt((x * x) + (y * y) + (z * z) + (w * w));
    }

    Vector4 Vector4::normalized() const {
        const float length = this->length();
        if (length < EPSILON) {
            return {};
        }
        return {x / length, y / length, z / length, w / length};
    }

    float Vector4::dot(const Vector4& other) const {
        return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
    }

    Vector4 Vector4::operator+(const Vector4& other) const {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    Vector4 Vector4::operator-(const Vector4& other) const {
        return {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    Vector4 Vector4::operator*(float value) const {
        return {x * value, y * value, z * value, w * value};
    }

    Vector4 Vector4::operator/(float value) const {
        return {x / value, y / value, z / value, w / value};
    }

    Vector4& Vector4::operator+=(const Vector4& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& Vector4::operator-=(const Vector4& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    bool Vector4::operator==(const Vector4& other) const {
        return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON && std::abs(z - other.z) < EPSILON &&
               std::abs(w - other.w) < EPSILON;
    }
} // namespace di_renderer::math
