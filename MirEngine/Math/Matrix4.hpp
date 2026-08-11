// MirEngine/Math/Matrix/Matrix4.hpp
// 🧮 Матрица 4×4 — главная матрица для 3D-преобразований.
//
// Matrix4 — это "швейцарский нож" компьютерной графики. Она умеет:
//   • Переносить объекты (translation).
//   • Вращать объекты (rotation).
//   • Масштабировать объекты (scale).
//   • Строить перспективу и ортографическую проекцию (камера).
//   • Комбинировать несколько преобразований в одно (умножение).
//
// В отличие от Matrix3, Matrix4 работает в однородных координатах:
// точка (x,y,z) представляется как Vector4(x,y,z,1.0),
// а направление — как Vector4(x,y,z,0.0).
// Благодаря этому одна матрица 4×4 может одновременно и повернуть,
// и перенести объект — что критически важно для всего рендеринга.
//
// Как читать матрицу (по строкам):
//   [ m00 m01 m02 m03 ]   <- строка 0: ось X и перенос X
//   [ m10 m11 m12 m13 ]   <- строка 1: ось Y и перенос Y
//   [ m20 m21 m22 m23 ]   <- строка 2: ось Z и перенос Z
//   [ m30 m31 m32 m33 ]   <- строка 3: перспектива (обычно 0,0,0,1)
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "../Vector/Vector3.hpp"         // mir::Vector3
#include "../Vector/Vector4.hpp"         // mir::Vector4
#include "../../Core/Types/Angle.hpp"    // mir::Angle
#include <array>                         // std::array
#include <cmath>                         // sin, cos, tan

namespace mir {

class Matrix4 {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт единичную матрицу (ничего не меняет).
    constexpr Matrix4() noexcept
        : m_data{1.0, 0.0, 0.0, 0.0,
                 0.0, 1.0, 0.0, 0.0,
                 0.0, 0.0, 1.0, 0.0,
                 0.0, 0.0, 0.0, 1.0}
    {}

    // Создаёт матрицу из 16 чисел (по строкам).
    constexpr Matrix4(
        Scalar m00, Scalar m01, Scalar m02, Scalar m03,
        Scalar m10, Scalar m11, Scalar m12, Scalar m13,
        Scalar m20, Scalar m21, Scalar m22, Scalar m23,
        Scalar m30, Scalar m31, Scalar m32, Scalar m33
    ) noexcept
        : m_data{m00, m01, m02, m03,
                 m10, m11, m12, m13,
                 m20, m21, m22, m23,
                 m30, m31, m32, m33}
    {}

    // ── Статические фабрики ──────────────────────────────────

    [[nodiscard]] static constexpr Matrix4 identity() noexcept {
        return Matrix4{};
    }

    // Матрица переноса (смещения на dx, dy, dz).
    [[nodiscard]] static Matrix4 translation(Scalar dx, Scalar dy, Scalar dz) noexcept {
        Matrix4 result;
        result(0, 3) = dx;
        result(1, 3) = dy;
        result(2, 3) = dz;
        return result;
    }

    // Матрица переноса из вектора.
    [[nodiscard]] static Matrix4 translation(const Vector3& v) noexcept {
        return translation(v.x, v.y, v.z);
    }

    // Матрица масштабирования.
    [[nodiscard]] static Matrix4 scale(Scalar sx, Scalar sy, Scalar sz) noexcept {
        Matrix4 result;
        result(0, 0) = sx;
        result(1, 1) = sy;
        result(2, 2) = sz;
        return result;
    }

    // Матрица масштабирования из вектора.
    [[nodiscard]] static Matrix4 scale(const Vector3& v) noexcept {
        return scale(v.x, v.y, v.z);
    }

    // Матрица вращения вокруг оси X (в радианах или градусах через Angle).
    [[nodiscard]] static Matrix4 rotationX(const Angle& angle) noexcept {
        Scalar c = std::cos(angle.radians());
        Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(1, 1) =  c;  result(1, 2) = -s;
        result(2, 1) =  s;  result(2, 2) =  c;
        return result;
    }

    // Матрица вращения вокруг оси Y.
    [[nodiscard]] static Matrix4 rotationY(const Angle& angle) noexcept {
        Scalar c = std::cos(angle.radians());
        Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(0, 0) =  c;  result(0, 2) =  s;
        result(2, 0) = -s;  result(2, 2) =  c;
        return result;
    }

    // Матрица вращения вокруг оси Z.
    [[nodiscard]] static Matrix4 rotationZ(const Angle& angle) noexcept {
        Scalar c = std::cos(angle.radians());
        Scalar s = std::sin(angle.radians());
        Matrix4 result;
        result(0, 0) =  c;  result(0, 1) = -s;
        result(1, 0) =  s;  result(1, 1) =  c;
        return result;
    }

    // ── Доступ к элементам ──────────────────────────────────
    [[nodiscard]] constexpr Scalar operator()(int row, int col) const noexcept {
        return m_data[static_cast<std::size_t>(row * 4 + col)];
    }
    [[nodiscard]] constexpr Scalar& operator()(int row, int col) noexcept {
        return m_data[static_cast<std::size_t>(row * 4 + col)];
    }

    // ── Транспонирование ────────────────────────────────────
    [[nodiscard]] constexpr Matrix4 transposed() const noexcept {
        return Matrix4{
            (*this)(0,0), (*this)(1,0), (*this)(2,0), (*this)(3,0),
            (*this)(0,1), (*this)(1,1), (*this)(2,1), (*this)(3,1),
            (*this)(0,2), (*this)(1,2), (*this)(2,2), (*this)(3,2),
            (*this)(0,3), (*this)(1,3), (*this)(2,3), (*this)(3,3)
        };
    }

    // ── Умножение матриц ─────────────────────────────────────
    friend constexpr Matrix4 operator*(const Matrix4& a, const Matrix4& b) noexcept {
        Matrix4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                result(row, col) = a(row, 0) * b(0, col) +
                                   a(row, 1) * b(1, col) +
                                   a(row, 2) * b(2, col) +
                                   a(row, 3) * b(3, col);
            }
        }
        return result;
    }

    // ── Применение матрицы к точке/вектору ──────────────────
    friend constexpr Vector4 operator*(const Matrix4& m, const Vector4& v) noexcept {
        return Vector4{
            m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z + m(0,3) * v.w,
            m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z + m(1,3) * v.w,
            m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z + m(2,3) * v.w,
            m(3,0) * v.x + m(3,1) * v.y + m(3,2) * v.z + m(3,3) * v.w
        };
    }

    // Удобный метод для преобразования Vector3 (точка, w=1.0).
    [[nodiscard]] Vector3 transformPoint(const Vector3& point) const noexcept {
        Vector4 result = (*this) * Vector4(point, 1.0);
        return Vector3(result.x, result.y, result.z);
    }

    // Удобный метод для преобразования Vector3 (направление, w=0.0).
    [[nodiscard]] Vector3 transformDirection(const Vector3& dir) const noexcept {
        Vector4 result = (*this) * Vector4(dir, 0.0);
        return Vector3(result.x, result.y, result.z);
    }

    // ── Обратная матрица (упрощённая для ортогональных матриц) ──
    [[nodiscard]] Matrix4 inverse() const noexcept;

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Matrix4& a, const Matrix4& b) noexcept {
        for (int i = 0; i < 16; ++i) {
            if (a.m_data[i] != b.m_data[i]) return false;
        }
        return true;
    }
    friend constexpr bool operator!=(const Matrix4& a, const Matrix4& b) noexcept {
        return !(a == b);
    }

private:
    std::array<Scalar, 16> m_data;   // числа хранятся по строкам
};

} // namespace mir