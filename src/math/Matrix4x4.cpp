#include "Matrix4x4.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace di_renderer::math {
    Matrix4x4::Matrix4x4() {
        m_data.fill(0.0F);
    }

    Matrix4x4::Matrix4x4(const std::array<float, 16>& values) : m_data(values) {}

    Matrix4x4::Matrix4x4(const std::function<float(int, int)>& func) {
        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                (*this)(row, col) = func(row, col);
            }
        }
    }

    const float* Matrix4x4::data() const {
        return m_data.data();
    }

    Matrix4x4 Matrix4x4::identity() {
        return Matrix4x4([](int row, int col) { return row == col ? 1.0F : 0.0F; });
    }

    Matrix4x4 Matrix4x4::transposed() const {
        return Matrix4x4([this](int row, int col) { return (*this)(col, row); }); // NOLINT
    }

    float Matrix4x4::operator()(const size_t row, const size_t col) const {
        if (row >= 4 || col >= 4) {
            throw std::out_of_range("Couldn't get matrix element, parameters must be 0 < {par, par} < 4");
        }
        return m_data[(col * 4) + row]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    float& Matrix4x4::operator()(const size_t row, const size_t col) {
        if (row >= 4 || col >= 4) {
            throw std::out_of_range("Couldn't set matrix element,parameters must be 0 < (par, par) < 4");
        }
        return m_data[(col * 4) + row]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
        return Matrix4x4([this, &other](int row, int col) { return (*this)(row, col) + other(row, col); });
    }

    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
        return Matrix4x4([this, &other](int row, int col) { return (*this)(row, col) - other(row, col); });
    }

    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
        return Matrix4x4([this, &other](int row, int col) {
            float result = 0;
            for (int k = 0; k < 4; k++) {
                result += (*this)(row, k) * other(k, col);
            }
            return result;
        });
    }

    Vector4 Matrix4x4::operator*(const Vector4& vec) const {
        return {((*this)(0, 0) * vec.x) + ((*this)(0, 1) * vec.y) + ((*this)(0, 2) * vec.z) + ((*this)(0, 3) * vec.w),
                ((*this)(1, 0) * vec.x) + ((*this)(1, 1) * vec.y) + ((*this)(1, 2) * vec.z) + ((*this)(1, 3) * vec.w),
                ((*this)(2, 0) * vec.x) + ((*this)(2, 1) * vec.y) + ((*this)(2, 2) * vec.z) + ((*this)(2, 3) * vec.w),
                ((*this)(3, 0) * vec.x) + ((*this)(3, 1) * vec.y) + ((*this)(3, 2) * vec.z) + ((*this)(3, 3) * vec.w)};
    }

    Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& other) {
        *this = *this + other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& other) {
        *this = *this - other;
        return *this;
    }

    Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& other) {
        *this = *this * other;
        return *this;
    }

    bool Matrix4x4::operator==(const Matrix4x4& other) const {
        for (size_t row = 0; row < 4; ++row) {
            for (size_t col = 0; col < 4; ++col) {
                if (std::abs((*this)(row, col) - other(row, col)) > std::numeric_limits<float>::epsilon()) {
                    return false;
                }
            }
        }
        return true;
    }

    float Matrix4x4::calculate_determinant_3x3(float m00, float m01, float m02, float m10, float m11, float m12,
                                               float m20, float m21, float m22) {
        return (m00 * (m11 * m22 - m12 * m21)) - (m01 * (m10 * m22 - m12 * m20)) + (m02 * (m10 * m21 - m11 * m20));
    }

    float Matrix4x4::get_cofactor(int skip_row, int skip_col) const {
        std::array<int, 3> rows = {};
        std::array<int, 3> cols = {};

        int k = 0;
        for (int row = 0; row < 4; ++row) {
            if (row != skip_row) {
                rows[k++] = row; // NOLINT
            }
        }

        k = 0;
        for (int col = 0; col < 4; ++col) {
            if (col != skip_col) {
                cols[k++] = col; // NOLINT
            }
        }

        const float det =
            calculate_determinant_3x3((*this)(rows[0], cols[0]), (*this)(rows[0], cols[1]), (*this)(rows[0], cols[2]),
                                      (*this)(rows[1], cols[0]), (*this)(rows[1], cols[1]), (*this)(rows[1], cols[2]),
                                      (*this)(rows[2], cols[0]), (*this)(rows[2], cols[1]), (*this)(rows[2], cols[2]));

        return ((skip_row + skip_col) % 2 == 0) ? det : -det;
    }

    float Matrix4x4::determinant() const {
        float det = 0.0F;
        for (int col = 0; col < 4; ++col) {
            det += (*this)(0, col) * Matrix4x4::get_cofactor(0, col);
        }
        return det;
    }

    Matrix4x4 Matrix4x4::inverse() const {
        const float det = determinant();

        if (std::abs(det) < std::numeric_limits<float>::epsilon()) {
            return identity();
        }

        const float inv_det = 1.0F / det;
        Matrix4x4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float cofactor = Matrix4x4::get_cofactor(row, col);
                result(col, row) = cofactor * inv_det; // NOLINT
            }
        }
        return result;
    }

} // namespace di_renderer::math
