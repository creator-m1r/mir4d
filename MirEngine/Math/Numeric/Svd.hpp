// MirEngine/Math/Numeric/Svd.hpp
// 🧮 Сингулярное разложение (SVD) — устойчивый МНК и понижение ранга.
//
// Тонкий SVD матрицы A (m×n):
//   A ≈ U · diag(σ) · Vᵀ,
//   U — m×k ортонормированные столбцы,
//   V — n×k ортонормированные столбцы,
//   σ — k = min(m,n) неотрицательных сингулярных чисел (по убыванию).
//
// Реализация — односторонний метод Якоби (ортогонализация столбцов A),
// численно устойчивее, чем AᵀA. При m < n вход транспонируется.
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

/// Результат тонкого SVD: A ≈ U · diag(sigma) · Vᵀ.
struct SVD
{
    std::vector<std::vector<Scalar>> U;   // m × k
    std::vector<Scalar> sigma;            // k = min(m, n)
    std::vector<std::vector<Scalar>> V;   // n × k
};

[[nodiscard]] inline Scalar columnNorm2(
    const std::vector<std::vector<Scalar>>& M,
    std::size_t col,
    std::size_t rows)
{
    Scalar s = Scalar(0);
    for (std::size_t r = 0; r < rows; ++r)
        s += M[r][col] * M[r][col];
    return std::sqrt(s);
}

[[nodiscard]] inline Scalar columnDot(
    const std::vector<std::vector<Scalar>>& M,
    std::size_t c1,
    std::size_t c2,
    std::size_t rows)
{
    Scalar s = Scalar(0);
    for (std::size_t r = 0; r < rows; ++r)
        s += M[r][c1] * M[r][c2];
    return s;
}

/// Тонкий SVD матрицы A (поддерживает m ≥ n и m < n).
[[nodiscard]] inline mir4d::Result<SVD> svdDecompose(const std::vector<std::vector<Scalar>>& A)
{
    if (A.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");

    const std::size_t m = A.size();
    const std::size_t n = A.front().size();
    for (const auto& row : A)
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Строки разной длины");

    // При m < n — транспонируем и меняем U, V местами.
    if (m < n)
    {
        std::vector<std::vector<Scalar>> At(n, std::vector<Scalar>(m, Scalar(0)));
        for (std::size_t i = 0; i < m; ++i)
            for (std::size_t j = 0; j < n; ++j)
                At[j][i] = A[i][j];

        auto sub = svdDecompose(At); // At: n × m, n ≥ m
        if (!sub)
            return sub;
        // At = Ua · Σ · Vaᵀ  ⇒  A = Va · Σ · Uaᵀ
        return mir4d::success(SVD{sub.value().V, sub.value().sigma, sub.value().U});
    }

    // Случай m ≥ n: односторонний Якоби по столбцам A.
    std::vector<std::vector<Scalar>> U = A;
    std::vector<std::vector<Scalar>> V(n, std::vector<Scalar>(n, Scalar(0)));
    for (std::size_t i = 0; i < n; ++i)
        V[i][i] = Scalar(1);

    const Scalar tol = Scalar(1e-12);
    const int maxSweeps = 60;

    for (int sweep = 0; sweep < maxSweeps; ++sweep)
    {
        Scalar maxOff = Scalar(0);

        for (std::size_t p = 0; p < n; ++p)
        {
            for (std::size_t q = p + 1; q < n; ++q)
            {
                const Scalar alpha = columnDot(U, p, p, m);
                const Scalar beta = columnDot(U, q, q, m);
                const Scalar gamma = columnDot(U, p, q, m);

                const Scalar denom = std::sqrt(alpha * beta);
                if (denom <= tol)
                    continue;

                const Scalar rel = gamma / denom;
                maxOff = std::max(maxOff, std::abs(rel));
                if (std::abs(gamma) < tol * denom)
                    continue;

                const Scalar tau = (beta - alpha) / (Scalar(2) * gamma);
                const Scalar t = (tau >= Scalar(0) ? Scalar(1) : Scalar(-1)) /
                    (std::abs(tau) + std::sqrt(tau * tau + Scalar(1)));
                const Scalar c = Scalar(1) / std::sqrt(t * t + Scalar(1));
                const Scalar s = t * c;

                for (std::size_t r = 0; r < m; ++r)
                {
                    const Scalar up = U[r][p];
                    const Scalar uq = U[r][q];
                    U[r][p] = c * up - s * uq;
                    U[r][q] = s * up + c * uq;
                }
                for (std::size_t r = 0; r < n; ++r)
                {
                    const Scalar vp = V[r][p];
                    const Scalar vq = V[r][q];
                    V[r][p] = c * vp - s * vq;
                    V[r][q] = s * vp + c * vq;
                }
            }
        }

        if (maxOff < tol)
            break;
    }

    std::vector<Scalar> sigma(n, Scalar(0));
    for (std::size_t i = 0; i < n; ++i)
    {
        sigma[i] = columnNorm2(U, i, m);
        if (sigma[i] > tol)
        {
            for (std::size_t r = 0; r < m; ++r)
                U[r][i] /= sigma[i];
        }
    }

    // Сортировка по убыванию.
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t best = i;
        for (std::size_t j = i + 1; j < n; ++j)
            if (sigma[j] > sigma[best])
                best = j;
        if (best != i)
        {
            std::swap(sigma[i], sigma[best]);
            for (std::size_t r = 0; r < m; ++r)
                std::swap(U[r][i], U[r][best]);
            for (std::size_t r = 0; r < n; ++r)
                std::swap(V[r][i], V[r][best]);
        }
    }

    return mir4d::success(SVD{std::move(U), std::move(sigma), std::move(V)});
}

/// Решает задачу линейных наименьших квадратов min‖A·x − b‖₂ методом SVD.
/// Работает для переопределённых (m > n) и вырожденных систем (нулевые
/// сингулярные числа отбрасываются → минимально-нормальное решение).
/// A — m×n, b — длины m; возвращает x длины n.
[[nodiscard]] inline mir4d::Result<std::vector<Scalar>> solveLeastSquaresSVD(
    const std::vector<std::vector<Scalar>>& A,
    const std::vector<Scalar>& b)
{
    if (A.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");
    const std::size_t m = A.size();
    const std::size_t n = A.front().size();
    if (b.size() != m)
        return fail(mir4d::ErrorCode::InvalidArgument, "Размер b не совпадает с числом строк A");

    auto d = svdDecompose(A);
    if (!d)
        return std::unexpected(d.error());
    const SVD& s = d.value();

    const Scalar tol = (s.sigma.empty() ? Scalar(0) : s.sigma.front()) * Scalar(1e-12);

    // y = Uᵀ·b  (k-вектор, k = min(m, n))
    const std::size_t k = s.sigma.size();
    std::vector<Scalar> y(k, Scalar(0));
    for (std::size_t i = 0; i < k; ++i)
    {
        Scalar acc = Scalar(0);
        for (std::size_t r = 0; r < m; ++r)
            acc += s.U[r][i] * b[r];
        y[i] = acc;
    }

    // z = Σ⁻¹·y (отбрасываем нулевые сингулярные числа)
    std::vector<Scalar> z(k, Scalar(0));
    for (std::size_t i = 0; i < k; ++i)
        if (s.sigma[i] > tol)
            z[i] = y[i] / s.sigma[i];

    // x = V·z
    std::vector<Scalar> x(n, Scalar(0));
    for (std::size_t j = 0; j < n; ++j)
        for (std::size_t i = 0; i < k; ++i)
            x[j] += s.V[j][i] * z[i];

    return mir4d::success(std::move(x));
}

} // namespace mir::math
