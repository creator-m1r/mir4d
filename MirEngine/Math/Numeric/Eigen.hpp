// MirEngine/Math/Numeric/Eigen.hpp
// 🧮 Собственные значения симметричной матрицы — метод вращений Якоби.
//
// Применяется в механике (главные напряжения/деформации), PCA и
// модальном анализе. Матрица должна быть симметричной (A = Aᵀ).
//
// Возвращает mir4d::Result с парой:
//   eigenvalues  — вектор из n значений (по убыванию);
//   eigenvectors — eigenvectors[i] — i-й собственный вектор (длины n).
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

/// Собственные значения/векторы симметричной матрицы A методом Якоби.
[[nodiscard]] inline mir4d::Result<std::pair<std::vector<Scalar>, std::vector<std::vector<Scalar>>>>
symmetricEigen(const std::vector<std::vector<Scalar>>& A)
{
    const std::size_t n = A.size();

    if (n == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Пустая матрица");

    for (const auto& row : A)
    {
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Матрица не квадратная");
    }

    // Рабочая копия и матрица собственных векторов (единичная).
    std::vector<std::vector<Scalar>> D = A;
    std::vector<std::vector<Scalar>> V(n, std::vector<Scalar>(n, Scalar(0)));
    for (std::size_t i = 0; i < n; ++i)
        V[i][i] = Scalar(1);

    const Scalar tolerance = Scalar(1e-12);
    const int maxSweeps = 100;

    for (int sweep = 0; sweep < maxSweeps; ++sweep)
    {
        // Оценка внедиагональной нормы.
        Scalar off = Scalar(0);
        for (std::size_t p = 0; p < n; ++p)
            for (std::size_t q = p + 1; q < n; ++q)
                off += D[p][q] * D[p][q];
        off = std::sqrt(off);

        if (off < tolerance)
            break;

        for (std::size_t p = 0; p < n; ++p)
        {
            for (std::size_t q = p + 1; q < n; ++q)
            {
                const Scalar apq = D[p][q];
                if (std::abs(apq) < tolerance)
                    continue;

                const Scalar app = D[p][p];
                const Scalar aqq = D[q][q];

                // Тангенс угла вращения.
                const Scalar tau = (aqq - app) / (Scalar(2) * apq);
                const Scalar t = (tau >= Scalar(0) ? Scalar(1) : Scalar(-1)) /
                    (std::abs(tau) + std::sqrt(tau * tau + Scalar(1)));
                const Scalar c = Scalar(1) / std::sqrt(t * t + Scalar(1));
                const Scalar s = t * c;

                // Новые диагональные элементы.
                D[p][p] = c * c * app - Scalar(2) * s * c * apq + s * s * aqq;
                D[q][q] = s * s * app + Scalar(2) * s * c * apq + c * c * aqq;
                D[p][q] = D[q][p] = Scalar(0);

                // Вращение остальных строк/столбцов.
                for (std::size_t k = 0; k < n; ++k)
                {
                    if (k == p || k == q)
                        continue;
                    const Scalar akp = D[k][p];
                    const Scalar akq = D[k][q];
                    D[k][p] = c * akp - s * akq;
                    D[p][k] = D[k][p];
                    D[k][q] = s * akp + c * akq;
                    D[q][k] = D[k][q];
                }

                // Накопление вращения в собственных векторах.
                for (std::size_t k = 0; k < n; ++k)
                {
                    const Scalar vkp = V[k][p];
                    const Scalar vkq = V[k][q];
                    V[k][p] = c * vkp - s * vkq;
                    V[k][q] = s * vkp + c * vkq;
                }
            }
        }
    }

    // Сборка результата с сортировкой по убыванию собственных значений.
    std::vector<Scalar> eigenvalues(n);
    std::vector<std::vector<Scalar>> eigenvectors(n, std::vector<Scalar>(n));

    for (std::size_t i = 0; i < n; ++i)
    {
        eigenvalues[i] = D[i][i];
        for (std::size_t k = 0; k < n; ++k)
            eigenvectors[i][k] = V[k][i]; // i-й собственный вектор
    }

    // Простая сортировка выбором по убыванию.
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t best = i;
        for (std::size_t j = i + 1; j < n; ++j)
            if (eigenvalues[j] > eigenvalues[best])
                best = j;
        if (best != i)
        {
            std::swap(eigenvalues[i], eigenvalues[best]);
            std::swap(eigenvectors[i], eigenvectors[best]);
        }
    }

    return mir4d::success(std::make_pair(std::move(eigenvalues), std::move(eigenvectors)));
}

} // namespace mir::math
