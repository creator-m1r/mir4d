// MirEngine/Math/Matrix2.hpp
// 🧮 Матрица 2×2 — для линейных преобразований на плоскости.
//
// Matrix2 хранит 4 числа (2 строки × 2 столбца) и умеет:
//   • Поворачивать векторы на плоскости (вращение).
//   • Масштабировать векторы (изменение размера).
//   • Комбинировать несколько преобразований в одно (умножение матриц).
//   • Находить обратное преобразование (inverse).
//   • Вычислять определитель (площадь, на которую влияет матрица).
//
// Матрица 2×2 широко используется в 2D-графике, эскизах (Sketch),
// обработке изображений и везде, где нужны линейные преобразования
// на плоскости (X, Y).
//
// Как читать матрицу (по строкам):
//   [ m00 m01 ]   <- строка 0: преобразование по X
//   [ m10 m11 ]   <- строка 1: преобразование по Y
//
// Умножение матрицы на вектор-столбец (x, y):
//   newX = m00 * x + m01 * y
//   newY = m10 * x + m11 * y
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Core/Types/Scalar.hpp"      // mir::Scalar = double
#include "Vector/PlanarVector.hpp"      // mir::Vector2
#include <array>                         // std::array для хранения 4 чисел
#include <cmath>                         // std::abs для проверки на ноль
#include <algorithm>                     // std::swap (хотя не нужен, но на всякий случай)

namespace mir {

class Matrix2 {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт единичную матрицу (по диагонали 1.0, остальные 0.0).
    // Такая матрица при умножении на вектор не меняет его.
    constexpr Matrix2() noexcept
        : m_data{1.0, 0.0,
                 0.0, 1.0}
    {}

    // Создаёт матрицу из 4 чисел (по строкам).
    // m00 — первая строка, первый столбец; m01 — первая строка, второй столбец;
    // m10 — вторая строка, первый столбец; m11 — вторая строка, второй столбец.
    constexpr Matrix2(Scalar m00, Scalar m01,
                      Scalar m10, Scalar m11) noexcept
        : m_data{m00, m01,
                 m10, m11}
    {}

    // ── Статические фабрики ──────────────────────────────────

    // Единичная матрица (ничего не меняет при умножении).
    [[nodiscard]] static constexpr Matrix2 identity() noexcept {
        return Matrix2{};
    }

    // Матрица вращения на заданный угол (против часовой стрелки).
    // Угол передаётся в радианах (можно использовать класс Angle).
    [[nodiscard]] static Matrix2 rotation(Scalar angleRadians) noexcept {
        Scalar c = std::cos(angleRadians);
        Scalar s = std::sin(angleRadians);
        return Matrix2{
            c, -s,
            s,  c
        };
    }

    // Матрица масштабирования: растягивает по X в sx раз, по Y в sy раз.
    [[nodiscard]] static constexpr Matrix2 scale(Scalar sx, Scalar sy) noexcept {
        return Matrix2{
            sx, 0.0,
            0.0, sy
        };
    }

    // ── Доступ к элементам ──────────────────────────────────
    // Доступ по строке и столбцу (row, col от 0 до 1).
    [[nodiscard]] constexpr Scalar operator()(int row, int col) const noexcept {
        return m_data[static_cast<std::size_t>(row * 2 + col)];
    }
    [[nodiscard]] constexpr Scalar& operator()(int row, int col) noexcept {
        return m_data[static_cast<std::size_t>(row * 2 + col)];
    }

    // ── Транспонирование (строки становятся столбцами) ──────
    [[nodiscard]] constexpr Matrix2 transposed() const noexcept {
        return Matrix2{
            (*this)(0,0), (*this)(1,0),
            (*this)(0,1), (*this)(1,1)
        };
    }

    // ── Определитель (det) — мера "площади" преобразования ────
    // Если det = 0, матрица вырожденная (нельзя обратить).
    [[nodiscard]] constexpr Scalar determinant() const noexcept {
        return (*this)(0,0) * (*this)(1,1) - (*this)(0,1) * (*this)(1,0);
    }

    // ── Обратная матрица ────────────────────────────────────
    // Если det == 0, возвращает единичную матрицу (заглушка).
    [[nodiscard]] Matrix2 inverse() const noexcept {
        Scalar det = determinant();
        if (std::abs(det) < 1e-20) {
            return identity();   // вырожденная матрица — возвращаем единичную
        }
        Scalar invDet = 1.0 / det;
        return Matrix2{
             (*this)(1,1) * invDet, -(*this)(0,1) * invDet,
            -(*this)(1,0) * invDet,  (*this)(0,0) * invDet
        };
    }

    // ── Умножение матриц ─────────────────────────────────────
    friend constexpr Matrix2 operator*(const Matrix2& a, const Matrix2& b) noexcept {
        return Matrix2{
            a(0,0) * b(0,0) + a(0,1) * b(1,0),   // m00
            a(0,0) * b(0,1) + a(0,1) * b(1,1),   // m01
            a(1,0) * b(0,0) + a(1,1) * b(1,0),   // m10
            a(1,0) * b(0,1) + a(1,1) * b(1,1)    // m11
        };
    }

    // ── Умножение матрицы на вектор-столбец ─────────────────
    friend constexpr Vector2 operator*(const Matrix2& m, const Vector2& v) noexcept {
        return Vector2{
            m(0,0) * v.x + m(0,1) * v.y,
            m(1,0) * v.x + m(1,1) * v.y
        };
    }

    // ── Сравнение ────────────────────────────────────────────
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
    std::array<Scalar, 4> m_data;   // числа хранятся по строкам: [row0][col0], [row0][col1], [row1][col0], [row1][col1]
};

} // namespace mir