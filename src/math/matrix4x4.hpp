#pragma once

#include "vector4.hpp"

class Matrix4x4 {
public:
    float m[4][4];

    Matrix4x4();

    static Matrix4x4 identity();
    Matrix4x4 transposed() const;

    Matrix4x4 operator+(const Matrix4x4& other) const;
    Matrix4x4 operator-(const Matrix4x4& other) const;
    Matrix4x4 operator*(const Matrix4x4& other) const;

    Vector4 operator*(const Vector4& vec) const;
};

