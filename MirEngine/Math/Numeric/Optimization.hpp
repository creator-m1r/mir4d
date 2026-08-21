// MirEngine/Math/Numeric/Optimization.hpp
// 🧮 Многомерная оптимизация и нелинейные системы — ядро CAE/FEM.
//
// Содержит:
//   • solveNonlinearSystem — многомерный метод Ньютона–Рафсона
//     для системы уравнений F(x) = 0 (шагает через solveLinearSystem).
//   • minimizeGradientDescent — градиентный спуск по градиенту g(x).
//   • minimizeNewton         — Ньютоновская минимизация по grad + Hessian.
//
// Все методы принимают вектор-функции и возвращают mir4d::Result.
// Это основа для решения остаточных уравнений МКЭ и безусловной
// оптимизации параметров.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"
#include "LinearSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
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

using VectorN = std::vector<Scalar>;
using MatrixN = std::vector<std::vector<Scalar>>;
using FunctionN = std::function<VectorN(const VectorN&)>;
using JacobianN = std::function<MatrixN(const VectorN&)>;
using GradientN = std::function<VectorN(const VectorN&)>;
using HessianN = std::function<MatrixN(const VectorN&)>;

[[nodiscard]] inline Scalar vectorNorm(const VectorN& v) noexcept
{
    Scalar acc = Scalar(0);
    for (const Scalar x : v)
        acc += x * x;
    return std::sqrt(acc);
}

// ═══════════════════════════════════════════════════════════════
//  Нелинейные системы уравнений
// ═══════════════════════════════════════════════════════════════

/// Решает F(x) = 0 многомерным методом Ньютона–Рафсона.
/// На каждой итерации решает J(x)·Δ = −F(x) и обновляет x += Δ.
[[nodiscard]] inline mir4d::Result<VectorN> solveNonlinearSystem(
    FunctionN f,
    JacobianN jacobian,
    VectorN x,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    for (int iter = 0; iter < maxIter; ++iter)
    {
        const VectorN fx = f(x);
        if (vectorNorm(fx) <= tol)
            return mir4d::success(x);

        const MatrixN jx = jacobian(x);
        VectorN rhs = fx;
        for (Scalar& v : rhs)
            v = -v;

        auto step = solveLinearSystem(jx, rhs);
        if (!step)
            return std::unexpected(step.error());

        Scalar maxStep = Scalar(0);
        for (std::size_t k = 0; k < x.size(); ++k)
        {
            x[k] += step.value()[k];
            maxStep = std::max(maxStep, std::abs(step.value()[k]));
        }

        if (maxStep <= tol)
            return mir4d::success(x);
    }

    return fail(mir4d::ErrorCode::Internal, "Нелинейная система не сошлась за maxIter");
}

// ═══════════════════════════════════════════════════════════════
//  Безусловная минимизация
// ═══════════════════════════════════════════════════════════════

/// Минимизирует f(x) градиентным спуском: x ← x − rate·g(x).
/// Останавливается при ‖g(x)‖ ≤ tol.
[[nodiscard]] inline mir4d::Result<VectorN> minimizeGradientDescent(
    GradientN gradient,
    VectorN x,
    Scalar rate = Scalar(0.01),
    Scalar tol = Scalar(1e-7),
    int maxIter = 100000)
{
    if (tol <= Scalar(0) || maxIter <= 0 || rate <= Scalar(0))
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные параметры спуска");

    for (int iter = 0; iter < maxIter; ++iter)
    {
        const VectorN g = gradient(x);
        if (vectorNorm(g) <= tol)
            return mir4d::success(x);

        for (std::size_t k = 0; k < x.size(); ++k)
            x[k] -= rate * g[k];
    }

    return fail(mir4d::ErrorCode::Internal, "Градиентный спуск не сошёлся за maxIter");
}

/// Минимизирует f(x) методом Ньютона: x ← x − H(x)⁻¹·g(x).
/// Требует градиент и гессиан; сходится за метрику ‖Δ‖ ≤ tol.
[[nodiscard]] inline mir4d::Result<VectorN> minimizeNewton(
    GradientN gradient,
    HessianN hessian,
    VectorN x,
    Scalar tol = Scalar(1e-9),
    int maxIter = 200)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    for (int iter = 0; iter < maxIter; ++iter)
    {
        const VectorN g = gradient(x);
        if (vectorNorm(g) <= tol)
            return mir4d::success(x);

        const MatrixN hx = hessian(x);
        VectorN rhs = g;
        for (Scalar& v : rhs)
            v = -v;

        auto step = solveLinearSystem(hx, rhs);
        if (!step)
            return std::unexpected(step.error());

        Scalar maxStep = Scalar(0);
        for (std::size_t k = 0; k < x.size(); ++k)
        {
            x[k] += step.value()[k];
            maxStep = std::max(maxStep, std::abs(step.value()[k]));
        }

        if (maxStep <= tol)
            return mir4d::success(x);
    }

    return fail(mir4d::ErrorCode::Internal, "Ньютон-минимизация не сошлась за maxIter");
}

using ObjectiveN = std::function<Scalar(const VectorN&)>;
using ResidualN = std::function<VectorN(const VectorN&)>;  // r(x) — вектор невязок

/// Минимизирует f(x) квазиньютоновским методом BFGS (обратная гессиана
/// аппроксимируется, требуется только градиент). Линейный поиск — Армихо.
[[nodiscard]] inline mir4d::Result<VectorN> minimizeBFGS(
    ObjectiveN objective,
    GradientN gradient,
    VectorN x,
    Scalar tol = Scalar(1e-8),
    int maxIter = 500)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    const std::size_t n = x.size();
    std::vector<std::vector<Scalar>> H(n, std::vector<Scalar>(n, Scalar(0)));
    for (std::size_t i = 0; i < n; ++i)
        H[i][i] = Scalar(1);

    VectorN g = gradient(x);
    auto matVecH = [&](const VectorN& v) {
        VectorN r(n, Scalar(0));
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t k = 0; k < n; ++k)
                r[i] += H[i][k] * v[k];
        return r;
    };

    for (int iter = 0; iter < maxIter; ++iter)
    {
        if (vectorNorm(g) <= tol)
            return mir4d::success(x);

        VectorN p(n, Scalar(0));
        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                p[i] -= H[i][j] * g[j];

        Scalar gDotP = Scalar(0);
        for (std::size_t i = 0; i < n; ++i)
            gDotP += g[i] * p[i];

        const Scalar fval = objective(x);
        Scalar alpha = Scalar(1);
        VectorN xp = x;
        while (alpha > Scalar(1e-14))
        {
            VectorN trial(n, Scalar(0));
            for (std::size_t i = 0; i < n; ++i)
                trial[i] = x[i] + alpha * p[i];
            if (objective(trial) <= fval + Scalar(1e-4) * alpha * gDotP)
            {
                xp = std::move(trial);
                break;
            }
            alpha *= Scalar(0.5);
        }

        const VectorN gnew = gradient(xp);
        VectorN s(n), y(n);
        Scalar ys = Scalar(0);
        for (std::size_t i = 0; i < n; ++i)
        {
            s[i] = xp[i] - x[i];
            y[i] = gnew[i] - g[i];
            ys += y[i] * s[i];
        }

        if (std::abs(ys) > Scalar(1e-14))
        {
            const Scalar rho = Scalar(1) / ys;
            const VectorN Hy = matVecH(y);
            Scalar yHy = Scalar(0);
            for (std::size_t i = 0; i < n; ++i)
                yHy += y[i] * Hy[i];

            std::vector<std::vector<Scalar>> Hnew(n, std::vector<Scalar>(n, Scalar(0)));
            for (std::size_t i = 0; i < n; ++i)
                for (std::size_t j = 0; j < n; ++j)
                {
                    const Scalar term = H[i][j]
                        - rho * (s[i] * Hy[j] + Hy[i] * s[j])
                        + rho * rho * s[i] * yHy * s[j];
                    Hnew[i][j] = term + rho * s[i] * s[j];
                }
            H = std::move(Hnew);
        }
        x = std::move(xp);
        g = std::move(gnew);
    }

    return fail(mir4d::ErrorCode::Internal, "BFGS не сошёлся за maxIter");
}

/// Решает задачу нелинейных наименьших квадратов: минимизирует
/// Φ(x) = ½·‖r(x)‖² методом Левенберга–Марквардта.
/// r(x) — вектор невязок, J(x) — его якобиан (строки = невязки,
/// столбцы = параметры). На каждом шаге решается система
/// (JᵀJ + λ·diag(JᵀJ))·Δ = −Jᵀr; λ растёт при ухудшении, падает при успехе.
[[nodiscard]] inline mir4d::Result<VectorN> solveNonlinearLeastSquares(
    ResidualN residuals,
    JacobianN jacobian,
    VectorN x,
    Scalar tol = Scalar(1e-9),
    int maxIter = 200)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar lambda = Scalar(1e-3);
    for (int iter = 0; iter < maxIter; ++iter)
    {
        const VectorN r = residuals(x);
        const std::size_t m = r.size();
        const std::size_t n = x.size();

        Scalar cost = Scalar(0);
        for (Scalar v : r)
            cost += v * v;
        if (std::sqrt(cost) <= tol)
            return mir4d::success(x);

        const MatrixN J = jacobian(x);
        // JtJ = Jᵀ·J  (n×n) и Jtr = Jᵀ·r  (n).
        MatrixN JtJ(n, VectorN(n, Scalar(0)));
        VectorN Jtr(n, Scalar(0));
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                Scalar s = Scalar(0);
                for (std::size_t k = 0; k < m; ++k)
                    s += J[k][i] * J[k][j];
                JtJ[i][j] = s;
            }
            Scalar t = Scalar(0);
            for (std::size_t k = 0; k < m; ++k)
                t += J[k][i] * r[k];
            Jtr[i] = t;
        }

        Scalar maxDiag = Scalar(0);
        for (std::size_t i = 0; i < n; ++i)
            maxDiag = std::max(maxDiag, std::abs(JtJ[i][i]));

        bool accepted = false;
        for (int attempt = 0; attempt < 6 && !accepted; ++attempt)
        {
            MatrixN A = JtJ;
            for (std::size_t i = 0; i < n; ++i)
                A[i][i] += lambda * (JtJ[i][i] + Scalar(1e-12));

            VectorN rhs = Jtr;
            for (Scalar& v : rhs)
                v = -v;

            auto step = solveLinearSystem(A, rhs);
            if (!step)
                return std::unexpected(step.error());

            VectorN xNew = x;
            for (std::size_t i = 0; i < n; ++i)
                xNew[i] += step.value()[i];

            const VectorN rNew = residuals(xNew);
            Scalar costNew = Scalar(0);
            for (Scalar v : rNew)
                costNew += v * v;

            if (costNew < cost)
            {
                x = std::move(xNew);
                lambda = std::max(lambda * Scalar(0.1), Scalar(1e-12));
                accepted = true;
            }
            else
            {
                lambda *= Scalar(10);
            }
            (void)maxDiag;
        }

        if (!accepted)
            return fail(mir4d::ErrorCode::Internal, "LM не смог улучшить решение (maxIter)");
    }

    return fail(mir4d::ErrorCode::Internal, "Нелинейный МНК не сошёлся за maxIter");
}

} // namespace mir::math
