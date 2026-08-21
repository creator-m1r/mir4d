
#pragma once

#include "LinearSystem.hpp"
#include "Statistics.hpp"

#include <complex>
#include <cmath>
#include <numbers>
#include <vector>

namespace mir::math
{

using Complex = std::complex<Scalar>;

[[nodiscard]] inline Complex evaluatePolynomialComplex(const std::vector<Scalar>& a, const Complex& x)
{
    Complex r = Complex(0);
    for (auto it = a.rbegin(); it != a.rend(); ++it)
        r = r * x + Complex(*it);
    return r;
}

[[nodiscard]] inline std::vector<Scalar> polynomialDerivative(const std::vector<Scalar>& a)
{
    if (a.size() < 2)
        return {Scalar(0)};
    std::vector<Scalar> d(a.size() - 1, Scalar(0));
    for (std::size_t i = 1; i < a.size(); ++i)
        d[i - 1] = a[i] * static_cast<Scalar>(i);
    return d;
}

[[nodiscard]] inline std::vector<Scalar> polynomialIntegral(const std::vector<Scalar>& a)
{
    std::vector<Scalar> I(a.size() + 1, Scalar(0));
    for (std::size_t i = 0; i < a.size(); ++i)
        I[i + 1] = a[i] / static_cast<Scalar>(i + 1);
    return I;
}

[[nodiscard]] inline mir4d::Result<std::vector<Scalar>> polynomialFit(
    const std::vector<Scalar>& xs,
    const std::vector<Scalar>& ys,
    std::size_t degree)
{
    if (xs.size() != ys.size() || xs.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Несовпадение длин xs/ys");
    if (degree == 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Степень должна быть ≥ 1");

    const std::size_t m = xs.size();
    const std::size_t n = degree + 1;
    std::vector<std::vector<Scalar>> A(m, std::vector<Scalar>(n, Scalar(1)));
    for (std::size_t i = 0; i < m; ++i)
    {
        Scalar xk = xs[i];
        for (std::size_t j = 1; j < n; ++j)
        {
            A[i][j] = xk;
            xk *= xs[i];
        }
    }
    auto sol = solveLeastSquares(A, ys);
    if (!sol)
        return sol;
    return sol;
}

[[nodiscard]] inline std::vector<Complex> polynomialRoots(const std::vector<Scalar>& coeffs)
{

    std::size_t deg = coeffs.size();
    while (deg > 0 && std::abs(coeffs[deg - 1]) < 1e-14)
        --deg;
    if (deg <= 1)
        return {};

    std::vector<Scalar> a(coeffs);
    const Scalar lead = a[deg - 1];
    for (auto& v : a)
        v /= lead;

    const std::size_t n = deg - 1;
    const Complex r0 = Complex(Scalar(0.4), Scalar(0.9));
    std::vector<Complex> z(n);
    for (std::size_t k = 0; k < n; ++k)
        z[k] = r0 * std::exp(Complex(Scalar(0), Scalar(2) * std::numbers::pi_v<Scalar> * static_cast<Scalar>(k) / static_cast<Scalar>(n)));

    const Scalar tol = Scalar(1e-12);
    const int maxIter = 500;
    for (int iter = 0; iter < maxIter; ++iter)
    {
        bool converged = true;
        for (std::size_t i = 0; i < n; ++i)
        {
            const Complex p = evaluatePolynomialComplex(a, z[i]);
            Complex denom = Complex(1);
            for (std::size_t j = 0; j < n; ++j)
                if (j != i)
                    denom *= (z[i] - z[j]);
            if (std::abs(denom) < Scalar(1e-300))
                continue;
            const Complex delta = p / denom;
            if (std::abs(delta) > tol)
                converged = false;
            z[i] -= delta;
        }
        if (converged)
            break;
    }
    return z;
}

}
