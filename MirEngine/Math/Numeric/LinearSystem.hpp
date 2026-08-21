// MirEngine/Math/Numeric/LinearSystem.hpp
// 🧮 Численное решение линейных систем — ядро инженерной математики MIR 4D.
//
// Содержит:
//   • solveLinearSystem — решение квадратной СЛАУ A·x = b
//     методом Гаусса–Жордана с частичным выбором ведущего элемента.
//   • solveLeastSquares — решение переопределённой СЛАУ методом
//     наименьших квадратов (нормальная система (AᵀA)x = Aᵀb).
//
// Обе функции возвращают mir4d::Result: при вырожденности матрицы или
// несогласованных размерах возвращается описание ошибки, а не «мусор».
//
// Представление матрицы: std::vector<std::vector<Scalar>> (построчно).
// Это естественный и читаемый формат для инженерных расчётов; для
// сверхбольших систем позже можно добавить разреженные форматы, не
// меняя контракт функций.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace mir::math
{

#ifndef MIR_MATH_NUMERIC_FAIL_DEFINED
#define MIR_MATH_NUMERIC_FAIL_DEFINED
/// Возвращает «неудачу» в виде std::unexpected<Error> для функций Result.
[[nodiscard]] inline auto fail(mir4d::ErrorCode code, std::string_view message)
{
    return std::unexpected(mir4d::Error(code, message));
}
#endif

/// Решает квадратную СЛАУ A·x = b методом Гаусса–Жордана
/// с частичным выбором ведущего элемента.
///
/// \param A квадратная матрица n×n (построчно).
/// \param b правые части длины n.
/// \return решение x или ошибку (неквадратная/несогласованная матрица,
///         вырожденная матрица).
[[nodiscard]] inline mir4d::Result<std::vector<Scalar>> solveLinearSystem(
    const std::vector<std::vector<Scalar>>& A,
    const std::vector<Scalar>& b)
{
    const std::size_t n = A.size();

    if (n == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица A");

    if (b.size() != n)
        return fail(mir4d::ErrorCode::InvalidArgument,
            "Размер b не совпадает с размером A");

    for (const auto& row : A)
    {
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument,
                "Матрица A не квадратная");
    }

    // Рабочая копия: расширенная матрица [M | x].
    std::vector<std::vector<Scalar>> M = A;
    std::vector<Scalar> x = b;
    const Scalar epsilon = Scalar(1e-12);

    for (std::size_t col = 0; col < n; ++col)
    {
        // Частичный выбор ведущего элемента по столбцу.
        std::size_t pivotRow = col;
        Scalar maxValue = std::abs(M[col][col]);

        for (std::size_t row = col + 1; row < n; ++row)
        {
            const Scalar value = std::abs(M[row][col]);
            if (value > maxValue)
            {
                maxValue = value;
                pivotRow = row;
            }
        }

        if (maxValue <= epsilon)
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Матрица вырожденная (нет решения)");

        if (pivotRow != col)
        {
            std::swap(M[col], M[pivotRow]);
            std::swap(x[col], x[pivotRow]);
        }

        // Нормализация ведущей строки.
        const Scalar diag = M[col][col];
        for (std::size_t c = col; c < n; ++c)
            M[col][c] /= diag;
        x[col] /= diag;

        // Исключение элементов вне ведущей строки.
        for (std::size_t row = 0; row < n; ++row)
        {
            if (row == col)
                continue;

            const Scalar factor = M[row][col];
            if (std::abs(factor) <= epsilon)
                continue;

            for (std::size_t c = col; c < n; ++c)
                M[row][c] -= factor * M[col][c];
            x[row] -= factor * x[col];
        }
    }

    return mir4d::success(std::move(x));
}

/// Решает переопределённую СЛАУ A·x = b (m ≥ n) методом наименьших квадратов.
///
/// Минимизирует ‖A·x − b‖² через нормальную систему (AᵀA)x = Aᵀb,
/// которая решается solveLinearSystem. Подходит для аппроксимации,
/// регрессии и сглаживания измерений.
///
/// \return решение x (длины n) или ошибку.
[[nodiscard]] inline mir4d::Result<std::vector<Scalar>> solveLeastSquares(
    const std::vector<std::vector<Scalar>>& A,
    const std::vector<Scalar>& b)
{
    const std::size_t m = A.size();

    if (m == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица A");

    const std::size_t n = A.front().size();

    for (const auto& row : A)
    {
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument,
                "Строки A имеют разную длину");
    }

    if (b.size() != m)
        return fail(mir4d::ErrorCode::InvalidArgument,
            "Размер b не совпадает с числом строк A");

    if (m < n)
        return fail(mir4d::ErrorCode::InvalidArgument,
            "Число уравнений меньше числа неизвестных (m < n)");

    // Построение нормальной системы (AᵀA)x = Aᵀb.
    std::vector<std::vector<Scalar>> ata(n, std::vector<Scalar>(n, Scalar(0)));
    std::vector<Scalar> atb(n, Scalar(0));

    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            Scalar sum = Scalar(0);
            for (std::size_t k = 0; k < m; ++k)
                sum += A[k][i] * A[k][j];
            ata[i][j] = sum;
        }

        Scalar sum = Scalar(0);
        for (std::size_t k = 0; k < m; ++k)
            sum += A[k][i] * b[k];
        atb[i] = sum;
    }

    return solveLinearSystem(ata, atb);
}

} // namespace mir::math
