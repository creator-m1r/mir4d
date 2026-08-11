// MirEngine/Math/Matrix/Matrix3.hpp
// 🧮 Матрица 3×3 — для линейных преобразований в трёхмерном пространстве.
//
// Matrix3 хранит 9 чисел (3 строки × 3 столбца) и умеет:
//   • Поворачивать векторы (вращение).
//   • Масштабировать векторы (изменение размера).
//   • Комбинировать несколько преобразований в одно (умножение матриц).
//   • Находить обратное преобразование (inverse).
//
// В отличие от Matrix4, Matrix3 не может хранить перенос (смещение),
// только линейные преобразования. Это делает её идеальной для:
//   • Хранения вращений (без позиции).
//   • Преобразования нормалей (они не должны смещаться).
//   • Расчёта моментов инерции и других физических величин.
//
// Чистый C++23, без внешних зависимостей.


#include "../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "../Vector/Vector3.hpp"         // mir::Vector3
#include <array>                         // std::array для хранения 9 чисел
#include <cassert>                       // assert

namespace mir {

class Matrix3 {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт единичную матрицу (по диагонали 1.0, остальные 0.0).
    constexpr Matrix3() noexcept
        : m_data{1.0, 0.0, 0.0,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0}
    {}

    // Создаёт матрицу из 9 чисел (по строкам).
    constexpr Matrix3(
        Scalar m00, Scalar m01, Scalar m02,
        Scalar m10, Scalar m11, Scalar m12,
        Scalar m20, Scalar m21, Scalar m22
    ) noexcept
        : m_data{m00, m01, m02,
                 m10, m11, m12,
                 m20, m21, m22}
    {}

    // ── Статические фабрики ──────────────────────────────────

    // Единичная матрица (ничего не меняет при умножении).
    [[nodiscard]] static constexpr Matrix3 identity() noexcept {
        return Matrix3{};
    }

    // ── Доступ к элементам ──────────────────────────────────
    // Доступ по строке и столбцу (row, col от 0 до 2).
    [[nodiscard]] constexpr Scalar operator()(int row, int col) const noexcept {
        return m_data[static_cast<std::size_t>(row * 3 + col)];
    }
    [[nodiscard]] constexpr Scalar& operator()(int row, int col) noexcept {
        return m_data[static_cast<std::size_t>(row * 3 + col)];
    }

    // ── Транспонирование (строки становятся столбцами) ──────
    [[nodiscard]] constexpr Matrix3 transposed() const noexcept {
        return Matrix3{
            (*this)(0,0), (*this)(1,0), (*this)(2,0),
            (*this)(0,1), (*this)(1,1), (*this)(2,1),
            (*this)(0,2), (*this)(1,2), (*this)(2,2)
        };
    }

    // ── Определитель (det) — мера "объёма" преобразования ────
    // Если det = 0, матрица вырожденная (нельзя обратить).
    [[nodiscard]] constexpr Scalar determinant() const noexcept {
        return (*this)(0,0) * ((*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1))
             - (*this)(0,1) * ((*this)(1,0) * (*this)(2,2) - (*this)(1,2) * (*this)(2,0))
             + (*this)(0,2) * ((*this)(1,0) * (*this)(2,1) - (*this)(1,1) * (*this)(2,0));
    }

    // ── Обратная матрица ────────────────────────────────────
    // Если det == 0, возвращает единичную матрицу (заглушка).
    [[nodiscard]] Matrix3 inverse() const noexcept {
        Scalar det = determinant();
        if (std::abs(det) < 1e-20) {
            return identity();   // вырожденная матрица — возвращаем единичную
        }
        Scalar invDet = 1.0 / det;

        return Matrix3{
            invDet * ((*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1)),
            invDet * ((*this)(0,2) * (*this)(2,1) - (*this)(0,1) * (*this)(2,2)),
            invDet * ((*this)(0,1) * (*this)(1,2) - (*this)(0,2) * (*this)(1,1)),
            invDet * ((*this)(1,2) * (*this)(2,0) - (*this)(1,0) * (*this)(2,2)),
            invDet * ((*this)(0,0) * (*this)(2,2) - (*this)(0,2) * (*this)(2,0)),
            invDet * ((*this)(0,2) * (*this)(1,0) - (*this)(0,0) * (*this)(1,2)),
            invDet * ((*this)(1,0) * (*this)(2,1) - (*this)(1,1) * (*this)(2,0)),
            invDet * ((*this)(0,1) * (*this)(2,0) - (*this)(0,0) * (*this)(2,1)),
            invDet * ((*this)(0,0) * (*this)(1,1) - (*this)(0,1) * (*this)(1,0))
        };
    }

    // ── Умножение матриц ─────────────────────────────────────
    friend constexpr Matrix3 operator*(const Matrix3& a, const Matrix3& b) noexcept {
        Matrix3 result;
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                result(row, col) = a(row, 0) * b(0, col) +
                                   a(row, 1) * b(1, col) +
                                   a(row, 2) * b(2, col);
            }
        }
        return result;
    }

    // ── Умножение матрицы на вектор ─────────────────────────
    friend constexpr Vector3 operator*(const Matrix3& m, const Vector3& v) noexcept {
        return Vector3{
            m(0,0) * v.x + m(0,1) * v.y + m(0,2) * v.z,
            m(1,0) * v.x + m(1,1) * v.y + m(1,2) * v.z,
            m(2,0) * v.x + m(2,1) * v.y + m(2,2) * v.z
        };
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Matrix3& a, const Matrix3& b) noexcept {
        for (int i = 0; i < 9; ++i) {
            if (a.m_data[i] != b.m_data[i]) return false;
        }
        return true;
    }
    friend constexpr bool operator!=(const Matrix3& a, const Matrix3& b) noexcept {
        return !(a == b);
    }

private:
    std::array<Scalar, 9> m_data;   // числа хранятся по строкам: [row0][col0], [row0][col1], ...
};

} // namespace mir