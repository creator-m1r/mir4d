
#pragma once

#include "LinearSystem.hpp"

#include <cmath>
#include <functional>
#include <vector>

namespace mir::math
{

struct Pde1DResult
{
    std::vector<Scalar> x;
    std::vector<std::vector<Scalar>> snapshots;
};

inline Pde1DResult solveHeat1D(
    Scalar alpha,
    Scalar L,
    Scalar T,
    std::size_t nx,
    std::size_t nt,
    std::function<Scalar(Scalar)> initial,
    Scalar leftBc = Scalar(0),
    Scalar rightBc = Scalar(0))
{
    Pde1DResult res;
    const std::size_t n = nx;
    const Scalar dx = L / static_cast<Scalar>(n - 1);
    const Scalar dt = T / static_cast<Scalar>(nt);
    const Scalar r = alpha * dt / (dx * dx);

    res.x.resize(n);
    for (std::size_t i = 0; i < n; ++i)
        res.x[i] = static_cast<Scalar>(i) * dx;

    std::vector<Scalar> u(n), un(n);
    for (std::size_t i = 0; i < n; ++i)
        u[i] = initial(res.x[i]);
    u[0] = leftBc;
    u[n - 1] = rightBc;

    res.snapshots.reserve(nt + 1);
    res.snapshots.push_back(u);

    for (std::size_t s = 0; s < nt; ++s)
    {
        for (std::size_t i = 1; i + 1 < n; ++i)
            un[i] = u[i] + r * (u[i + 1] - Scalar(2) * u[i] + u[i - 1]);
        un[0] = leftBc;
        un[n - 1] = rightBc;
        u = un;
        res.snapshots.push_back(u);
    }
    return res;
}

inline Pde1DResult solveWave1D(
    Scalar c,
    Scalar L,
    Scalar T,
    std::size_t nx,
    std::size_t nt,
    std::function<Scalar(Scalar)> u0,
    std::function<Scalar(Scalar)> v0 = [](Scalar) { return Scalar(0); },
    Scalar leftBc = Scalar(0),
    Scalar rightBc = Scalar(0))
{
    Pde1DResult res;
    const std::size_t n = nx;
    const Scalar dx = L / static_cast<Scalar>(n - 1);
    const Scalar dt = T / static_cast<Scalar>(nt);
    const Scalar C = c * dt / dx;

    res.x.resize(n);
    for (std::size_t i = 0; i < n; ++i)
        res.x[i] = static_cast<Scalar>(i) * dx;

    std::vector<Scalar> prev(n), cur(n), next(n);
    for (std::size_t i = 0; i < n; ++i)
        cur[i] = u0(res.x[i]);
    cur[0] = leftBc;
    cur[n - 1] = rightBc;

    for (std::size_t i = 1; i + 1 < n; ++i)
        prev[i] = cur[i] - dt * v0(res.x[i]);

    res.snapshots.reserve(nt + 1);
    res.snapshots.push_back(cur);

    for (std::size_t s = 0; s < nt; ++s)
    {
        for (std::size_t i = 1; i + 1 < n; ++i)
            next[i] = Scalar(2) * cur[i] - prev[i] +
                C * C * (cur[i + 1] - Scalar(2) * cur[i] + cur[i - 1]);
        next[0] = leftBc;
        next[n - 1] = rightBc;
        prev = cur;
        cur = next;
        res.snapshots.push_back(cur);
    }
    return res;
}

struct Pde2DResult
{
    std::vector<std::vector<Scalar>> u;
    int n = 0;
};

inline Pde2DResult solvePoisson2D(
    int n,
    std::function<Scalar(Scalar, Scalar)> source,
    std::function<Scalar(Scalar, Scalar)> boundary = [](Scalar, Scalar) { return Scalar(0); },
    Scalar omega = Scalar(1.9),
    Scalar tol = Scalar(1e-6),
    int maxIter = 50000)
{
    Pde2DResult res;
    res.n = n;
    const Scalar h = Scalar(1) / static_cast<Scalar>(n - 1);
    res.u.assign(static_cast<std::size_t>(n), std::vector<Scalar>(static_cast<std::size_t>(n), Scalar(0)));

    for (int i = 0; i < n; ++i)
    {
        const Scalar x = static_cast<Scalar>(i) * h;
        res.u[i][0] = boundary(x, Scalar(0));
        res.u[i][n - 1] = boundary(x, Scalar(1));
    }
    for (int j = 0; j < n; ++j)
    {
        const Scalar y = static_cast<Scalar>(j) * h;
        res.u[0][j] = boundary(Scalar(0), y);
        res.u[n - 1][j] = boundary(Scalar(1), y);
    }

    const Scalar h2 = h * h;
    for (int iter = 0; iter < maxIter; ++iter)
    {
        Scalar maxDelta = Scalar(0);
        for (int i = 1; i + 1 < n; ++i)
        {
            for (int j = 1; j + 1 < n; ++j)
            {
                const Scalar x = static_cast<Scalar>(i) * h;
                const Scalar y = static_cast<Scalar>(j) * h;
                const Scalar rhs = res.u[i - 1][j] + res.u[i + 1][j] + res.u[i][j - 1] + res.u[i][j + 1]
                    + h2 * source(x, y);
                const Scalar updated = (Scalar(1) - omega) * res.u[i][j] + (omega / Scalar(4)) * rhs;
                maxDelta = std::max(maxDelta, std::abs(updated - res.u[i][j]));
                res.u[i][j] = updated;
            }
        }
        if (maxDelta <= tol)
            break;
    }
    return res;
}

}
