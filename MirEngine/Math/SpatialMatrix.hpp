// MirEngine/Math/Matrix3.hpp
// 🧮 Матрица 3×3 — линейные преобразования в трёхмерном пространстве.
//
// C++23

#pragma once

#include "../Core/Types/Scalar.hpp"
#include "Vector/Vector.hpp"

#include <array>
#include <cmath>
#include <cstddef>

namespace mir
{

class Matrix3
{
public:
    constexpr Matrix3() noexcept
        : m_data{
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0}
    {
    }

    constexpr Matrix3(
        Scalar m00, Scalar m01, Scalar m02,
        Scalar m10, Scalar m11, Scalar m12,
        Scalar m20, Scalar m21, Scalar m22) noexcept
        : m_data{
            m00, m01, m02,
            m10, m11, m12,
            m20, m21, m22}
    {
    }

    [[nodiscard]] static constexpr Matrix3 identity() noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr Scalar operator()(
        std::size_t row,
        std::size_t col) const noexcept
    {
        return m_data[row * 3 + col];
    }

    [[nodiscard]] constexpr Scalar& operator()(
        std::size_t row,
        std::size_t col) noexcept
    {
        return m_data[row * 3 + col];
    }

    [[nodiscard]] constexpr Matrix3 transposed() const noexcept
    {
        return {
            (*this)(0, 0), (*this)(1, 0), (*this)(2, 0),
            (*this)(0, 1), (*this)(1, 1), (*this)(2, 1),
            (*this)(0, 2), (*this)(1, 2), (*this)(2, 2)};
    }

    [[nodiscard]] constexpr Scalar determinant() const noexcept
    {
        return
            (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1))
            - (*this)(0, 1) * ((*this)(1, 0) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 0))
            + (*this)(0, 2) * ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0));
    }

    [[nodiscard]] Matrix3 inverse(
        Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar det = determinant();

        if (std::abs(det) <= epsilon)
        {
            return identity();
        }

        const Scalar invDet = Scalar(1.0) / det;

        return {
            invDet * ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1)),
            invDet * ((*this)(0, 2) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 2)),
            invDet * ((*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1)),
            invDet * ((*this)(1, 2) * (*this)(2, 0) - (*this)(1, 0) * (*this)(2, 2)),
            invDet * ((*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0)),
            invDet * ((*this)(0, 2) * (*this)(1, 0) - (*this)(0, 0) * (*this)(1, 2)),
            invDet * ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0)),
            invDet * ((*this)(0, 1) * (*this)(2, 0) - (*this)(0, 0) * (*this)(2, 1)),
            invDet * ((*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0))};
    }

    friend constexpr Matrix3 operator*(
        const Matrix3& a,
        const Matrix3& b) noexcept
    {
        Matrix3 result;

        for (std::size_t row = 0; row < 3; ++row)
        {
            for (std::size_t col = 0; col < 3; ++col)
            {
                result(row, col) =
                    a(row, 0) * b(0, col) +
                    a(row, 1) * b(1, col) +
                    a(row, 2) * b(2, col);
            }
        }

        return result;
    }

    friend constexpr Vector3 operator*(
        const Matrix3& matrix,
        const Vector3& vector) noexcept
    {
        return {
            matrix(0, 0) * vector.x + matrix(0, 1) * vector.y + matrix(0, 2) * vector.z,
            matrix(1, 0) * vector.x + matrix(1, 1) * vector.y + matrix(1, 2) * vector.z,
            matrix(2, 0) * vector.x + matrix(2, 1) * vector.y + matrix(2, 2) * vector.z};
    }

    friend constexpr bool operator==(
        const Matrix3& a,
        const Matrix3& b) noexcept
    {
        return a.m_data == b.m_data;
    }

    friend constexpr bool operator!=(
        const Matrix3& a,
        const Matrix3& b) noexcept
    {
        return !(a == b);
    }

private:
    std::array<Scalar, 9> m_data{};
};

} // namespace mir
