// MirEngine/Math/Numeric/Decomposition.hpp
// 🧮 Матричные разложения — основа линейной алгебры MIR 4D.
//
// Содержит:
//   • luDecompose / luSolve — LU с частичным выбором ведущего элемента
//     (повторное решение для многих правых частей).
//   • cholesky            — разложение L·Lᵀ для симметрично-положительно
//     определённых (SPD) матриц (градиентный спуск, ковариации).
//   • qrDecompose         — QR (модифицированный Грам–Шмидт) для
//     устойчивого МНК и ортогонализации.
//
// Все методы возвращают mir4d::Result при вырожденности/несовпадении.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mir::math
{

#ifndef MIR_MATH_NUMERIC_FAIL_DEFINED
#define MIR_MATH_NUMERIC_FAIL_DEFINED
[[nodiscard]] inline auto fail(mir4d::ErrorCode code, std::string_view message)
{
    return std::unexpected(mir4d::Error(code, message));
}
#endif

using MatrixN = std::vector<std::vector<Scalar>>;
using VectorN = std::vector<Scalar>;

/// Результат LU-разложения: A = Pᵀ·L·U.
struct LUDecomposition
{
    MatrixN L;                  // Нижняя треугольная (единичная диагональ).
    MatrixN U;                  // Верхняя треугольная.
    std::vector<std::size_t> piv; // Перестановка строк: piv[i] — исходная строка i-й.
};

/// LU-разложение с частичным выбором ведущего элемента (Doolittle).
[[nodiscard]] inline mir4d::Result<LUDecomposition> luDecompose(const MatrixN& A)
{
    const std::size_t n = A.size();
    if (n == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");
    for (const auto& row : A)
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Матрица не квадратная");

    LUDecomposition lu;
    lu.U = A;
    lu.L = MatrixN(n, VectorN(n, Scalar(0)));
    lu.piv.assign(n, 0);
    for (std::size_t i = 0; i < n; ++i)
    {
        lu.L[i][i] = Scalar(1);
        lu.piv[i] = i;
    }

    const Scalar eps = Scalar(1e-12);

    for (std::size_t k = 0; k < n; ++k)
    {
        std::size_t pivotRow = k;
        Scalar maxVal = std::abs(lu.U[k][k]);
        for (std::size_t r = k + 1; r < n; ++r)
        {
            const Scalar v = std::abs(lu.U[r][k]);
            if (v > maxVal)
            {
                maxVal = v;
                pivotRow = r;
            }
        }
        if (maxVal <= eps)
            return fail(mir4d::ErrorCode::ValidationFailed, "Матрица вырожденная (LU)");

        if (pivotRow != k)
        {
            std::swap(lu.U[k], lu.U[pivotRow]);
            std::swap(lu.L[k], lu.L[pivotRow]);
            std::swap(lu.piv[k], lu.piv[pivotRow]);
        }

        for (std::size_t i = k + 1; i < n; ++i)
        {
            lu.L[i][k] = lu.U[i][k] / lu.U[k][k];
            for (std::size_t j = k; j < n; ++j)
                lu.U[i][j] -= lu.L[i][k] * lu.U[k][j];
        }
    }

    return mir4d::success(std::move(lu));
}

/// Решает A·x = b, используя ранее вычисленное LU-разложение.
[[nodiscard]] inline mir4d::Result<VectorN> luSolve(const LUDecomposition& lu, const VectorN& b)
{
    const std::size_t n = lu.L.size();
    if (b.size() != n)
        return fail(mir4d::ErrorCode::InvalidArgument, "Размер b не совпадает");

    // Переставленная правая часть.
    VectorN pb(n);
    for (std::size_t i = 0; i < n; ++i)
        pb[i] = b[lu.piv[i]];

    // Прямая подстановка: L·y = pb.
    VectorN y(n, Scalar(0));
    for (std::size_t i = 0; i < n; ++i)
    {
        Scalar s = pb[i];
        for (std::size_t j = 0; j < i; ++j)
            s -= lu.L[i][j] * y[j];
        y[i] = s; // L[i][i] == 1
    }

    // Обратная подстановка: U·x = y.
    VectorN x(n, Scalar(0));
    for (std::size_t i = n; i-- > 0;)
    {
        Scalar s = y[i];
        for (std::size_t j = i + 1; j < n; ++j)
            s -= lu.U[i][j] * x[j];
        x[i] = s / lu.U[i][i];
    }

    return mir4d::success(std::move(x));
}

/// Разложение Холесского L·Lᵀ для симметрично-положительно определённой
/// матрицы A. Возвращает нижнюю треугольную L.
[[nodiscard]] inline mir4d::Result<MatrixN> cholesky(const MatrixN& A)
{
    const std::size_t n = A.size();
    if (n == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");
    for (const auto& row : A)
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Матрица не квадратная");

    MatrixN L(n, VectorN(n, Scalar(0)));
    const Scalar eps = Scalar(1e-12);

    for (std::size_t j = 0; j < n; ++j)
    {
        Scalar diag = A[j][j];
        for (std::size_t k = 0; k < j; ++k)
            diag -= L[j][k] * L[j][k];

        if (diag <= eps)
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Матрица не SPD (Холесский невозможен)");

        L[j][j] = std::sqrt(diag);

        for (std::size_t i = j + 1; i < n; ++i)
        {
            Scalar sum = A[i][j];
            for (std::size_t k = 0; k < j; ++k)
                sum -= L[i][k] * L[j][k];
            L[i][j] = sum / L[j][j];
        }
    }

    return mir4d::success(std::move(L));
}

/// QR-разложение (модифицированный Грам–Шмидт) для матрицы A размера m×n (m ≥ n).
/// Возвращает Q (m×n ортонормированные столбцы) и R (n×n верхняя треугольная).
[[nodiscard]] inline mir4d::Result<std::pair<MatrixN, MatrixN>> qrDecompose(const MatrixN& A)
{
    if (A.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");

    const std::size_t m = A.size();
    const std::size_t n = A.front().size();
    for (const auto& row : A)
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Строки разной длины");

    if (m < n)
        return fail(mir4d::ErrorCode::InvalidArgument, "Требуется m ≥ n для QR");

    // Столбцы A.
    auto column = [&](std::size_t col) -> VectorN
    {
        VectorN v(m);
        for (std::size_t i = 0; i < m; ++i)
            v[i] = A[i][col];
        return v;
    };

    MatrixN Q(m, VectorN(n, Scalar(0)));
    MatrixN R(n, VectorN(n, Scalar(0)));
    const Scalar eps = Scalar(1e-12);

    for (std::size_t j = 0; j < n; ++j)
    {
        VectorN v = column(j);

        for (std::size_t i = 0; i < j; ++i)
        {
            Scalar dot = Scalar(0);
            for (std::size_t r = 0; r < m; ++r)
                dot += Q[r][i] * v[r];
            R[i][j] = dot;

            for (std::size_t r = 0; r < m; ++r)
                v[r] -= dot * Q[r][i];
        }

        Scalar norm = Scalar(0);
        for (std::size_t r = 0; r < m; ++r)
            norm += v[r] * v[r];
        norm = std::sqrt(norm);

        if (norm <= eps)
            return fail(mir4d::ErrorCode::ValidationFailed, "Линейно зависимые столбцы (QR)");

        R[j][j] = norm;
        for (std::size_t r = 0; r < m; ++r)
            Q[r][j] = v[r] / norm;
    }

    return mir4d::success(std::make_pair(std::move(Q), std::move(R)));
}

} // namespace mir::math
