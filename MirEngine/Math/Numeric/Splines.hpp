
#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string_view>
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

struct CubicSpline
{
    std::vector<Scalar> xs;
    std::vector<Scalar> a;
    std::vector<Scalar> b;
    std::vector<Scalar> c;
    std::vector<Scalar> d;
};

[[nodiscard]] inline mir4d::Result<CubicSpline> buildCubicSpline(
    std::vector<Scalar> xs,
    std::vector<Scalar> ys)
{
    const std::size_t n = xs.size();
    if (n < 2 || ys.size() != n)
        return fail(mir4d::ErrorCode::InvalidArgument, "Нужно ≥ 2 узлов и равные размеры x/y");

    for (std::size_t i = 1; i < n; ++i)
        if (xs[i] <= xs[i - 1])
            return fail(mir4d::ErrorCode::InvalidArgument, "Узлы x должны быть строго возрастающими");

    std::vector<Scalar> M(n, Scalar(0));
    if (n > 2)
    {

        std::vector<Scalar> low(n, Scalar(0));
        std::vector<Scalar> diag(n, Scalar(0));
        std::vector<Scalar> up(n, Scalar(0));
        std::vector<Scalar> rhs(n, Scalar(0));

        for (std::size_t i = 1; i < n - 1; ++i)
        {
            const Scalar h0 = xs[i] - xs[i - 1];
            const Scalar h1 = xs[i + 1] - xs[i];
            const Scalar slopeR = (ys[i + 1] - ys[i]) / h1;
            const Scalar slopeL = (ys[i] - ys[i - 1]) / h0;
            low[i]  = h0;
            diag[i] = Scalar(2) * (h0 + h1);
            up[i]   = h1;
            rhs[i]  = Scalar(6) * (slopeR - slopeL);
        }

        for (std::size_t i = 2; i < n - 1; ++i)
        {
            const Scalar w = low[i] / diag[i - 1];
            diag[i] -= w * up[i - 1];
            rhs[i]  -= w * rhs[i - 1];
        }
        M[n - 2] = rhs[n - 2] / diag[n - 2];
        for (std::size_t i = n - 2; i-- > 1; )
            M[i] = (rhs[i] - up[i] * M[i + 1]) / diag[i];
    }

    CubicSpline sp;
    sp.xs = xs;
    sp.a.resize(n - 1);
    sp.b.resize(n - 1);
    sp.c.resize(n - 1);
    sp.d.resize(n - 1);
    for (std::size_t i = 0; i + 1 < n; ++i)
    {
        const Scalar h = xs[i + 1] - xs[i];
        sp.a[i] = ys[i];
        sp.b[i] = (ys[i + 1] - ys[i]) / h - h * (Scalar(2) * M[i] + M[i + 1]) / Scalar(6);
        sp.c[i] = M[i] / Scalar(2);
        sp.d[i] = (M[i + 1] - M[i]) / (Scalar(6) * h);
    }
    return mir4d::success(std::move(sp));
}

[[nodiscard]] inline std::size_t splineInterval(const CubicSpline& sp, Scalar x) noexcept
{
    const auto it = std::upper_bound(sp.xs.begin(), sp.xs.end(), x);
    std::size_t i = static_cast<std::size_t>(it - sp.xs.begin());
    if (i == 0)
        return 0;
    if (i >= sp.xs.size() - 1)
        return sp.xs.size() - 2;
    return i - 1;
}

[[nodiscard]] inline Scalar evalSpline(const CubicSpline& sp, Scalar x) noexcept
{
    const std::size_t i = splineInterval(sp, x);
    const Scalar u = x - sp.xs[i];
    return sp.a[i] + u * (sp.b[i] + u * (sp.c[i] + u * sp.d[i]));
}

[[nodiscard]] inline Scalar evalSplineDerivative(const CubicSpline& sp, Scalar x) noexcept
{
    const std::size_t i = splineInterval(sp, x);
    const Scalar u = x - sp.xs[i];
    return sp.b[i] + u * (Scalar(2) * sp.c[i] + u * Scalar(3) * sp.d[i]);
}

}
