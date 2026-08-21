
#pragma once

#include "../Core/Types/Scalar.hpp"
#include "Vector/PlanarVector.hpp"
#include <array>
#include <cmath>
#include <algorithm>

namespace mir {

class Matrix2 {
public:

    constexpr Matrix2() noexcept
        : m_data{1.0, 0.0,
                 0.0, 1.0}
    {}

    constexpr Matrix2(Scalar m00, Scalar m01,
                      Scalar m10, Scalar m11) noexcept
        : m_data{m00, m01,
                 m10, m11}
    {}

    [[nodiscard]] static constexpr Matrix2 identity() noexcept {
        return Matrix2{};
    }

    [[nodiscard]] static Matrix2 rotation(Scalar angleRadians) noexcept {
        Scalar c = std::cos(angleRadians);
        Scalar s = std::sin(angleRadians);
        return Matrix2{
            c, -s,
            s,  c
        };
    }

    [[nodiscard]] static constexpr Matrix2 scale(Scalar sx, Scalar sy) noexcept {
        return Matrix2{
            sx, 0.0,
            0.0, sy
        };
    }

    [[nodiscard]] constexpr Scalar operator()(int row, int col) const noexcept {
        return m_data[static_cast<std::size_t>(row * 2 + col)];
    }
    [[nodiscard]] constexpr Scalar& operator()(int row, int col) noexcept {
        return m_data[static_cast<std::size_t>(row * 2 + col)];
    }

    [[nodiscard]] constexpr Matrix2 transposed() const noexcept {
        return Matrix2{
            (*this)(0,0), (*this)(1,0),
            (*this)(0,1), (*this)(1,1)
        };
    }

    [[nodiscard]] constexpr Scalar determinant() const noexcept {
        return (*this)(0,0) * (*this)(1,1) - (*this)(0,1) * (*this)(1,0);
    }

    [[nodiscard]] Matrix2 inverse() const noexcept {
        Scalar det = determinant();
        if (std::abs(det) < 1e-20) {
            return identity();
        }
        Scalar invDet = 1.0 / det;
        return Matrix2{
             (*this)(1,1) * invDet, -(*this)(0,1) * invDet,
            -(*this)(1,0) * invDet,  (*this)(0,0) * invDet
        };
    }

    friend constexpr Matrix2 operator*(const Matrix2& a, const Matrix2& b) noexcept {
        return Matrix2{
            a(0,0) * b(0,0) + a(0,1) * b(1,0),
            a(0,0) * b(0,1) + a(0,1) * b(1,1),
            a(1,0) * b(0,0) + a(1,1) * b(1,0),
            a(1,0) * b(0,1) + a(1,1) * b(1,1)
        };
    }

    friend constexpr Vector2 operator*(const Matrix2& m, const Vector2& v) noexcept {
        return Vector2{
            m(0,0) * v.x + m(0,1) * v.y,
            m(1,0) * v.x + m(1,1) * v.y
        };
    }

    friend constexpr bool operator==(const Matrix2& a, const Matrix2& b) noexcept {
        for (int i = 0; i < 4; ++i) {
            if (a.m_data[i] != b.m_data[i]) return false;
        }
        return true;
    }
    friend constexpr bool operator!=(const Matrix2& a, const Matrix2& b) noexcept {
        return !(a == b);
    }

private:
    std::array<Scalar, 4> m_data;
};

}