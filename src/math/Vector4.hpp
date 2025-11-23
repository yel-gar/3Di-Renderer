#pragma once

namespace di_renderer::math
{
    class Vector4
    {
      public:
        static constexpr float EPS = 1e-8;
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

        Vector4& operator+=(const Vector4& other);
        Vector4& operator-=(const Vector4& other);

        bool operator==(const Vector4& other) const;
    };
} // namespace di_renderer::math
