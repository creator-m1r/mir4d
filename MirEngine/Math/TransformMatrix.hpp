// MirEngine/Math/Matrix4.hpp
// 🧮 Матрица 4×4 — основа 3D-преобразований MirEngine.
//
// Хранение: по строкам.
// Вектор трактуется как столбец:
//   result = matrix * vector
//
// C++23

#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Core/Types/Angle.hpp"
#include "Vector/Vector.hpp"
#include "Vector/HomogeneousVector.hpp"

#include <array>
#include <cmath>
#include <cstddef>

namespace mir
{

class Matrix4
{
public:
    constexpr Matrix4() noexcept
        : m_data{
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0}
    {
    }

    constexpr Matrix4(
        Scalar m00, Scalar m01, Scalar m02, Scalar m03,
        Scalar m10, Scalar m11, Scalar m12, Scalar m13,
        Scalar m20, Scalar m21, Scalar m22, Scalar m23,
        Scalar m30, Scalar m31, Scalar m32, Scalar m33) noexcept
        : m_data{
            m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33}
    {
    }

    [[nodiscard]] static constexpr Matrix4 identity() noexcept
    {
        return {};
    }

    [[nodiscard]] static constexpr Matrix4 translation(
        Scalar dx,
        Scalar dy,
        Scalar dz) noexcept
    {
        Matrix4 result;
        result(0, 3) = dx;
        result(1, 3) = dy;
        result(2, 3) = dz;
        return result;
    }

    [[nodiscard]] static constexpr Matrix4 translation(
        const Vector3& value) noexcept
    {
        return translation(value.x, value.y, value.z);
    }

    [[nodiscard]] static constexpr Matrix4 scale(
        Scalar sx,
        Scalar sy,
        Scalar sz) noexcept
    {
        Matrix4 result;
        result(0, 0) = sx;
        result(1, 1) = sy;
        result(2, 2) = sz;
        return result;
    }

    [[nodiscard]] static constexpr Matrix4 scale(
        const Vector3& value) noexcept
    {
        return scale(value.x, value.y, value.z);
    }

    [[nodiscard]] static Matrix4 rotationX(const Angle& angle) noexcept
    {
        const Scalar c = std::cos(angle.radians());
        const Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(1, 1) = c;
        result(1, 2) = -s;
        result(2, 1) = s;
        result(2, 2) = c;
        return result;
    }

    [[nodiscard]] static Matrix4 rotationY(const Angle& angle) noexcept
    {
        const Scalar c = std::cos(angle.radians());
        const Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(0, 0) = c;
        result(0, 2) = s;
        result(2, 0) = -s;
        result(2, 2) = c;
        return result;
    }

    [[nodiscard]] static Matrix4 rotationZ(const Angle& angle) noexcept
    {
        const Scalar c = std::cos(angle.radians());
        const Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(0, 0) = c;
        result(0, 1) = -s;
        result(1, 0) = s;
        result(1, 1) = c;
        return result;
    }

    [[nodiscard]] constexpr Scalar operator()(
        std::size_t row,
        std::size_t col) const noexcept
    {
        return m_data[row * 4 + col];
    }

    [[nodiscard]] constexpr Scalar& operator()(
        std::size_t row,
        std::size_t col) noexcept
    {
        return m_data[row * 4 + col];
    }

    [[nodiscard]] constexpr Matrix4 transposed() const noexcept
    {
        return {
            (*this)(0, 0), (*this)(1, 0), (*this)(2, 0), (*this)(3, 0),
            (*this)(0, 1), (*this)(1, 1), (*this)(2, 1), (*this)(3, 1),
            (*this)(0, 2), (*this)(1, 2), (*this)(2, 2), (*this)(3, 2),
            (*this)(0, 3), (*this)(1, 3), (*this)(2, 3), (*this)(3, 3)};
    }

    friend constexpr Matrix4 operator*(
        const Matrix4& a,
        const Matrix4& b) noexcept
    {
        Matrix4 result;
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t col = 0; col < 4; ++col)
            {
                result(row, col) =
                    a(row, 0) * b(0, col) +
                    a(row, 1) * b(1, col) +
                    a(row, 2) * b(2, col) +
                    a(row, 3) * b(3, col);
            }
        }
        return result;
    }

    friend constexpr Vector4 operator*(
        const Matrix4& matrix,
        const Vector4& vector) noexcept
    {
        return {
            matrix(0, 0) * vector.x + matrix(0, 1) * vector.y +
                matrix(0, 2) * vector.z + matrix(0, 3) * vector.w,
            matrix(1, 0) * vector.x + matrix(1, 1) * vector.y +
                matrix(1, 2) * vector.z + matrix(1, 3) * vector.w,
            matrix(2, 0) * vector.x + matrix(2, 1) * vector.y +
                matrix(2, 2) * vector.z + matrix(2, 3) * vector.w,
            matrix(3, 0) * vector.x + matrix(3, 1) * vector.y +
                matrix(3, 2) * vector.z + matrix(3, 3) * vector.w};
    }

    [[nodiscard]] Vector3 transformPoint(const Vector3& point) const noexcept
    {
        const Vector4 result = *this * Vector4(point, Scalar(1.0));
        if (std::abs(result.w) > Scalar(1e-20) &&
            std::abs(result.w - Scalar(1.0)) > Scalar(1e-20))
        {
            const Scalar invW = Scalar(1.0) / result.w;
            return {result.x * invW, result.y * invW, result.z * invW};
        }
        return {result.x, result.y, result.z};
    }

    [[nodiscard]] Vector3 transformDirection(const Vector3& direction) const noexcept
    {
        const Vector4 result = *this * Vector4(direction, Scalar(0.0));
        return {result.x, result.y, result.z};
    }

    [[nodiscard]] Matrix4 inverse(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        Scalar augmented[4][8]{};
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t col = 0; col < 4; ++col)
                augmented[row][col] = (*this)(row, col);
            augmented[row][row + 4] = Scalar(1.0);
        }

        for (std::size_t pivotColumn = 0; pivotColumn < 4; ++pivotColumn)
        {
            std::size_t pivotRow = pivotColumn;
            Scalar maxValue = std::abs(augmented[pivotRow][pivotColumn]);

            for (std::size_t row = pivotColumn + 1; row < 4; ++row)
            {
                const Scalar value = std::abs(augmented[row][pivotColumn]);
                if (value > maxValue)
                {
                    maxValue = value;
                    pivotRow = row;
                }
            }

            if (maxValue <= epsilon)
                return identity();

            if (pivotRow != pivotColumn)
            {
                for (std::size_t col = 0; col < 8; ++col)
                {
                    const Scalar temporary = augmented[pivotColumn][col];
                    augmented[pivotColumn][col] = augmented[pivotRow][col];
                    augmented[pivotRow][col] = temporary;
                }
            }

            const Scalar pivot = augmented[pivotColumn][pivotColumn];
            for (std::size_t col = 0; col < 8; ++col)
                augmented[pivotColumn][col] /= pivot;

            for (std::size_t row = 0; row < 4; ++row)
            {
                if (row == pivotColumn)
                    continue;

                const Scalar factor = augmented[row][pivotColumn];
                if (std::abs(factor) <= epsilon)
                    continue;

                for (std::size_t col = 0; col < 8; ++col)
                    augmented[row][col] -= factor * augmented[pivotColumn][col];
            }
        }

        Matrix4 result;
        for (std::size_t row = 0; row < 4; ++row)
            for (std::size_t col = 0; col < 4; ++col)
                result(row, col) = augmented[row][col + 4];
        return result;
    }

    friend constexpr bool operator==(const Matrix4& a, const Matrix4& b) noexcept
    {
        return a.m_data == b.m_data;
    }

    friend constexpr bool operator!=(const Matrix4& a, const Matrix4& b) noexcept
    {
        return !(a == b);
    }

private:
    std::array<Scalar, 16> m_data{};
};

} // namespace mir
