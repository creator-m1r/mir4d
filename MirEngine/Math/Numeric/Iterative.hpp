
#pragma once

#include "LinearSystem.hpp"

#include <cstddef>
#include <functional>
#include <vector>

namespace mir::math
{

[[nodiscard]] inline Scalar innerProduct(const VectorN& a, const VectorN& b)
{
    Scalar s = Scalar(0);
    for (std::size_t i = 0; i < a.size(); ++i)
        s += a[i] * b[i];
    return s;
}

[[nodiscard]] inline VectorN matVec(const MatrixN& A, const VectorN& x)
{
    VectorN y(A.size(), Scalar(0));
    for (std::size_t i = 0; i < A.size(); ++i)
    {
        Scalar s = Scalar(0);
        for (std::size_t j = 0; j < x.size(); ++j)
            s += A[i][j] * x[j];
        y[i] = s;
    }
    return y;
}

[[nodiscard]] inline VectorN conjugateGradient(
    const MatrixN& A,
    const VectorN& b,
    std::size_t maxIter = 1000,
    Scalar tol = Scalar(1e-10))
{
    const std::size_t n = b.size();
    VectorN x(n, Scalar(0));
    VectorN r = b;
    VectorN p = r;
    Scalar rsold = innerProduct(r, r);

    for (std::size_t k = 0; k < maxIter; ++k)
    {
        if (rsold < tol * tol)
            break;
        const VectorN Ap = matVec(A, p);
        const Scalar denom = innerProduct(p, Ap);
        const Scalar alpha = rsold / denom;
        for (std::size_t i = 0; i < n; ++i)
        {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }
        const Scalar rsnew = innerProduct(r, r);
        if (rsnew < tol * tol)
            break;
        const Scalar beta = rsnew / rsold;
        for (std::size_t i = 0; i < n; ++i)
            p[i] = r[i] + beta * p[i];
        rsold = rsnew;
    }
    return x;
}

[[nodiscard]] inline VectorN conjugateGradient(
    const std::function<VectorN(const VectorN&)>& matvec,
    const VectorN& b,
    std::size_t maxIter = 1000,
    Scalar tol = Scalar(1e-10))
{
    const std::size_t n = b.size();
    VectorN x(n, Scalar(0));
    VectorN r = b;
    VectorN p = r;
    Scalar rsold = innerProduct(r, r);
    for (std::size_t k = 0; k < maxIter; ++k)
    {
        if (rsold < tol * tol)
            break;
        const VectorN Ap = matvec(p);
        const Scalar denom = innerProduct(p, Ap);
        const Scalar alpha = rsold / denom;
        for (std::size_t i = 0; i < n; ++i)
        {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }
        const Scalar rsnew = innerProduct(r, r);
        if (rsnew < tol * tol)
            break;
        const Scalar beta = rsnew / rsold;
        for (std::size_t i = 0; i < n; ++i)
            p[i] = r[i] + beta * p[i];
        rsold = rsnew;
    }
    return x;
}

[[nodiscard]] inline mir4d::Result<VectorN> gaussSeidel(
    const MatrixN& A,
    const VectorN& b,
    std::size_t maxIter = 1000,
    Scalar tol = Scalar(1e-10),
    VectorN x0 = {})
{
    const std::size_t n = b.size();
    if (A.size() != n)
        return fail(mir4d::ErrorCode::InvalidArgument, "Несовпадение размеров A/b");
    for (const auto& row : A)
        if (row.size() != n)
            return fail(mir4d::ErrorCode::InvalidArgument, "Матрица не квадратная");

    VectorN x = x0.empty() ? VectorN(n, Scalar(0)) : x0;
    for (std::size_t iter = 0; iter < maxIter; ++iter)
    {
        Scalar maxDiff = Scalar(0);
        for (std::size_t i = 0; i < n; ++i)
        {
            if (std::abs(A[i][i]) < Scalar(1e-14))
                return fail(mir4d::ErrorCode::ValidationFailed, "Нулевой диагональный элемент");
            Scalar sigma = b[i];
            for (std::size_t j = 0; j < n; ++j)
                if (j != i)
                    sigma -= A[i][j] * x[j];
            const Scalar newX = sigma / A[i][i];
            maxDiff = std::max(maxDiff, std::abs(newX - x[i]));
            x[i] = newX;
        }
        if (maxDiff < tol)
            break;
    }
    return mir4d::success(std::move(x));
}

[[nodiscard]] inline mir4d::Result<VectorN> jacobiSolve(
    const MatrixN& A,
    const VectorN& b,
    std::size_t maxIter = 1000,
    Scalar tol = Scalar(1e-10),
    VectorN x0 = {})
{
    const std::size_t n = b.size();
    if (A.size() != n)
        return fail(mir4d::ErrorCode::InvalidArgument, "Несовпадение размеров A/b");

    VectorN x = x0.empty() ? VectorN(n, Scalar(0)) : x0;
    VectorN xnew(n, Scalar(0));
    for (std::size_t iter = 0; iter < maxIter; ++iter)
    {
        Scalar maxDiff = Scalar(0);
        for (std::size_t i = 0; i < n; ++i)
        {
            if (std::abs(A[i][i]) < Scalar(1e-14))
                return fail(mir4d::ErrorCode::ValidationFailed, "Нулевой диагональный элемент");
            Scalar sigma = b[i];
            for (std::size_t j = 0; j < n; ++j)
                if (j != i)
                    sigma -= A[i][j] * x[j];
            xnew[i] = sigma / A[i][i];
        }
        for (std::size_t i = 0; i < n; ++i)
            maxDiff = std::max(maxDiff, std::abs(xnew[i] - x[i]));
        x = xnew;
        if (maxDiff < tol)
            break;
    }
    return mir4d::success(std::move(x));
}

[[nodiscard]] inline VectorN vscale(Scalar a, const VectorN& x)
{
    VectorN y(x.size());
    for (std::size_t i = 0; i < x.size(); ++i)
        y[i] = a * x[i];
    return y;
}

[[nodiscard]] inline VectorN gmres(
    const std::function<VectorN(const VectorN&)>& A,
    const VectorN& b,
    std::size_t restart = 30,
    std::size_t maxIter = 500,
    Scalar tol = Scalar(1e-10))
{
    const std::size_t n = b.size();
    VectorN x(n, Scalar(0));
    VectorN r = b;
    Scalar beta0 = std::sqrt(innerProduct(r, r));
    if (beta0 < tol)
        return x;

    const Scalar bnrm = (beta0 > Scalar(0)) ? beta0 : Scalar(1);

    std::vector<VectorN> V(restart + 1, VectorN(n, Scalar(0)));
    std::vector<Scalar> cs(restart, Scalar(0)), sn(restart, Scalar(0)), g(restart + 1, Scalar(0));

    for (std::size_t total = 0; total < maxIter;)
    {
        Scalar beta = std::sqrt(innerProduct(r, r));
        V[0] = vscale(Scalar(1) / beta, r);
        g.assign(restart + 1, Scalar(0));
        g[0] = beta;

        std::size_t j = 0;
        std::vector<std::vector<Scalar>> H(restart + 1, std::vector<Scalar>(restart, Scalar(0)));

        for (; j < restart && total + j < maxIter; ++j)
        {
            VectorN w = A(V[j]);
            for (std::size_t i = 0; i <= j; ++i)
            {
                H[i][j] = innerProduct(V[i], w);
                for (std::size_t k = 0; k < n; ++k)
                    w[k] -= H[i][j] * V[i][k];
            }
            const Scalar hNext = std::sqrt(innerProduct(w, w));
            H[j + 1][j] = hNext;
            if (hNext > Scalar(1e-14))
                V[j + 1] = vscale(Scalar(1) / hNext, w);

            for (std::size_t i = 0; i < j; ++i)
            {
                const Scalar tmp = cs[i] * H[i][j] - sn[i] * H[i + 1][j];
                H[i + 1][j] = sn[i] * H[i][j] + cs[i] * H[i + 1][j];
                H[i][j] = tmp;
            }
            const Scalar hj = H[j][j];
            const Scalar hjp = H[j + 1][j];
            const Scalar denom = std::sqrt(hj * hj + hjp * hjp);
            cs[j] = hj / denom;
            sn[j] = hjp / denom;
            H[j][j] = denom;
            H[j + 1][j] = Scalar(0);

            const Scalar gNew = cs[j] * g[j] - sn[j] * g[j + 1];
            g[j + 1] = sn[j] * g[j] + cs[j] * g[j + 1];
            g[j] = gNew;

            if (std::abs(g[j + 1]) < tol * bnrm)
            {
                ++j;
                break;
            }
        }

        const std::size_t m = j;
        std::vector<Scalar> y(m, Scalar(0));
        for (std::size_t i2 = m; i2-- > 0;)
        {
            Scalar s = g[i2];
            for (std::size_t k2 = i2 + 1; k2 < m; ++k2)
                s -= H[i2][k2] * y[k2];
            y[i2] = s / H[i2][i2];
        }
        for (std::size_t i2 = 0; i2 < m; ++i2)
            for (std::size_t k = 0; k < n; ++k)
                x[k] += V[i2][k] * y[i2];

        total += m;
        VectorN Ax = A(x);
        for (std::size_t k = 0; k < n; ++k)
            r[k] = b[k] - Ax[k];
        if (std::sqrt(innerProduct(r, r)) < tol * bnrm)
            break;
    }
    return x;
}

[[nodiscard]] inline VectorN bicgstab(
    const std::function<VectorN(const VectorN&)>& A,
    const VectorN& b,
    std::size_t maxIter = 500,
    Scalar tol = Scalar(1e-10),
    VectorN x0 = {})
{
    const std::size_t n = b.size();
    VectorN x = x0.empty() ? VectorN(n, Scalar(0)) : std::move(x0);
    VectorN r = b;
    {
        const VectorN Ax = A(x);
        for (std::size_t i = 0; i < n; ++i)
            r[i] -= Ax[i];
    }
    const Scalar bnrm = std::max(std::sqrt(innerProduct(b, b)), Scalar(1));

    VectorN r0hat = r;
    VectorN v(n, Scalar(0));
    VectorN p(n, Scalar(0));
    Scalar rho = Scalar(1), alpha = Scalar(1), omega = Scalar(1);

    for (std::size_t it = 0; it < maxIter; ++it)
    {
        const Scalar rhoOld = rho;
        rho = innerProduct(r0hat, r);
        if (std::abs(rho) < Scalar(1e-30))
            break;
        const Scalar beta = (rho / rhoOld) * (alpha / omega);
        for (std::size_t i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        const VectorN vp = A(p);
        alpha = rho / innerProduct(r0hat, vp);
        VectorN s(n);
        for (std::size_t i = 0; i < n; ++i)
            s[i] = r[i] - alpha * vp[i];
        if (std::sqrt(innerProduct(s, s)) < tol * bnrm)
        {
            for (std::size_t i = 0; i < n; ++i)
                x[i] += alpha * p[i];
            break;
        }
        const VectorN t = A(s);
        omega = innerProduct(t, s) / innerProduct(t, t);
        for (std::size_t i = 0; i < n; ++i)
            x[i] += alpha * p[i] + omega * s[i];
        v = vp;
        for (std::size_t i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];
        if (std::sqrt(innerProduct(r, r)) < tol * bnrm)
            break;
    }
    return x;
}

}
