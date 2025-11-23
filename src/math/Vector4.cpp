#include "Vector4.hpp"

#include <cmath>

namespace di_renderer::math
{
    constexpr float EPS = 1e-8;

    Vector4::Vector4() : x(0.0F), y(0.0F), z(0.0F), w(0.0F) {}

    Vector4::Vector4(const float x, const float y, const float z, const float w) : x(x), y(y), z(z), w(w) {}

    float Vector4::length() const
    {
        return std::sqrt((x * x) + (y * y) + (z * z) + (w * w));
    }

    Vector4 Vector4::normalized() const
    {
        const float length = this->length();
        if (length < EPS)
        {
            return {0.0F, 0.0F, 0.0F, 0.0F};
        }
        return {x / length, y / length, z / length, w / length};
    }

    float Vector4::dot(const Vector4& other) const
    {
        return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
    }

    Vector4 Vector4::operator+(const Vector4& other) const
    {
        return {x + other.x, y + other.y, z + other.z, w + other.w};
    }

    Vector4 Vector4::operator-(const Vector4& other) const
    {
        return {x - other.x, y - other.y, z - other.z, w - other.w};
    }

    Vector4& Vector4::operator+=(const Vector4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    Vector4& Vector4::operator-=(const Vector4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    bool Vector4::operator==(const Vector4& other) const
    {
        return std::abs(x - other.x) < EPS && std::abs(y - other.y) < EPS && std::abs(z - other.z) < EPS &&
               std::abs(w - other.w) < EPS;
    }
} // namespace di_renderer::math
