#pragma once
#include <array>
#include "Vector4.hpp"

class Matrix4x4 {
public:
    void set(const std::array<float,16>& values);
    Matrix4x4();
    
    static Matrix4x4 identity();
    Matrix4x4 transposed() const;

    float get(int row, int col) const;
    void set_element(int row, int col, float value);

    Matrix4x4 operator+(const Matrix4x4& other) const;
    Matrix4x4 operator-(const Matrix4x4& other) const;
    Matrix4x4 operator*(const Matrix4x4& other) const;

    Vector4 operator*(const Vector4& vec) const;
private:
    std::array<float,16> m_;
};

