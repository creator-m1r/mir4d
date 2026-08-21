
#pragma once

#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <cstddef>
#include <functional>
#include <vector>

namespace mir::math
{

using ScalarFunction = std::function<Scalar(const std::vector<Scalar>&)>;
using VectorFunction = std::function<std::vector<Scalar>(const std::vector<Scalar>&)>;

[[nodiscard]] inline std::vector<Scalar> numericalGradient(
    ScalarFunction f,
    const std::vector<Scalar>& x,
    Scalar h = Scalar(1e-5))
{
    const std::size_t n = x.size();
    std::vector<Scalar> grad(n, Scalar(0));
    std::vector<Scalar> xp = x;
    std::vector<Scalar> xm = x;

    for (std::size_t i = 0; i < n; ++i)
    {
        const Scalar saved = x[i];
        xp[i] = saved + h;
        xm[i] = saved - h;
        grad[i] = (f(xp) - f(xm)) / (Scalar(2) * h);
        xp[i] = xm[i] = saved;
    }

    return grad;
}

[[nodiscard]] inline std::vector<std::vector<Scalar>> numericalJacobian(
    VectorFunction f,
    const std::vector<Scalar>& x,
    Scalar h = Scalar(1e-5))
{
    const std::size_t n = x.size();
    const std::vector<Scalar> fx = f(x);
    const std::size_t m = fx.size();

    std::vector<std::vector<Scalar>> J(m, std::vector<Scalar>(n, Scalar(0)));
    std::vector<Scalar> xp = x;
    std::vector<Scalar> xm = x;

    for (std::size_t j = 0; j < n; ++j)
    {
        const Scalar saved = x[j];
        xp[j] = saved + h;
        xm[j] = saved - h;

        const std::vector<Scalar> fp = f(xp);
        const std::vector<Scalar> fm = f(xm);
        for (std::size_t i = 0; i < m; ++i)
            J[i][j] = (fp[i] - fm[i]) / (Scalar(2) * h);

        xp[j] = xm[j] = saved;
    }

    return J;
}

[[nodiscard]] inline std::vector<std::vector<Scalar>> numericalHessian(
    ScalarFunction f,
    const std::vector<Scalar>& x,
    Scalar h = Scalar(1e-5))
{
    const std::size_t n = x.size();
    std::vector<std::vector<Scalar>> H(n, std::vector<Scalar>(n, Scalar(0)));

    for (std::size_t i = 0; i < n; ++i)
    {
        std::vector<Scalar> xp = x;
        std::vector<Scalar> xm = x;
        xp[i] += h;
        xm[i] -= h;
        H[i][i] = (f(xp) - Scalar(2) * f(x) + f(xm)) / (h * h);
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = i + 1; j < n; ++j)
        {
            std::vector<Scalar> pp = x;
            std::vector<Scalar> pm = x;
            std::vector<Scalar> mp = x;
            std::vector<Scalar> mm = x;

            pp[i] += h; pp[j] += h;
            pm[i] += h; pm[j] -= h;
            mp[i] -= h; mp[j] += h;
            mm[i] -= h; mm[j] -= h;

            const Scalar value =
                (f(pp) - f(pm) - f(mp) + f(mm)) / (Scalar(4) * h * h);
            H[i][j] = value;
            H[j][i] = value;
        }
    }

    return H;
}

}
