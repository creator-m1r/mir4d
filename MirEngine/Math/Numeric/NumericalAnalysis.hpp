
#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"

#include <cmath>
#include <functional>
#include <utility>

namespace mir::math
{

#ifndef MIR_MATH_NUMERIC_FAIL_DEFINED
#define MIR_MATH_NUMERIC_FAIL_DEFINED

[[nodiscard]] inline auto fail(mir4d::ErrorCode code, std::string_view message)
{
    return std::unexpected(mir4d::Error(code, message));
}
#endif

[[nodiscard]] inline mir4d::Result<Scalar> findRootBisection(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar fa = f(a);
    Scalar fb = f(b);

    if (fa == Scalar(0))
        return mir4d::success(a);
    if (fb == Scalar(0))
        return mir4d::success(b);

    if (fa * fb > Scalar(0))
        return fail(mir4d::ErrorCode::ValidationFailed,
            "Корень не зажат: f(a) и f(b) одного знака");

    Scalar lo = a;
    Scalar hi = b;

    for (int i = 0; i < maxIter; ++i)
    {
        const Scalar mid = (lo + hi) * Scalar(0.5);
        const Scalar fm = f(mid);

        if (std::abs(fm) <= tol || (hi - lo) * Scalar(0.5) <= tol)
            return mir4d::success(mid);

        if (fa * fm <= Scalar(0))
        {
            hi = mid;
            fb = fm;
        }
        else
        {
            lo = mid;
            fa = fm;
        }
    }

    return fail(mir4d::ErrorCode::Internal, "Бисекция не сошлась за maxIter");
}

[[nodiscard]] inline mir4d::Result<Scalar> findRootNewton(
    std::function<Scalar(Scalar)> f,
    std::function<Scalar(Scalar)> df,
    Scalar x0,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar x = x0;

    for (int i = 0; i < maxIter; ++i)
    {
        const Scalar fx = f(x);
        if (std::abs(fx) <= tol)
            return mir4d::success(x);

        const Scalar d = df(x);
        if (std::abs(d) <= Scalar(1e-14))
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Производная близка к нулю (сингулярность)");

        const Scalar next = x - fx / d;
        if (std::abs(next - x) <= tol)
            return mir4d::success(next);

        x = next;
    }

    return fail(mir4d::ErrorCode::Internal, "Метод Ньютона не сошёлся за maxIter");
}

[[nodiscard]] inline mir4d::Result<Scalar> findRootSecant(
    std::function<Scalar(Scalar)> f,
    Scalar x0,
    Scalar x1,
    Scalar tol = Scalar(1e-9),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    Scalar xPrev = x0;
    Scalar fPrev = f(x0);

    if (std::abs(fPrev) <= tol)
        return mir4d::success(x0);

    Scalar x = x1;
    Scalar fx = f(x1);

    for (int i = 0; i < maxIter; ++i)
    {
        if (std::abs(fx) <= tol)
            return mir4d::success(x);

        const Scalar denom = fx - fPrev;
        if (std::abs(denom) <= Scalar(1e-14))
            return fail(mir4d::ErrorCode::ValidationFailed,
                "Знаменатель метода секущих близок к нулю");

        const Scalar next = x - fx * (x - xPrev) / denom;
        if (std::abs(next - x) <= tol)
            return mir4d::success(next);

        xPrev = x;
        fPrev = fx;
        x = next;
        fx = f(x);
    }

    return fail(mir4d::ErrorCode::Internal, "Метод секущих не сошёлся за maxIter");
}

[[nodiscard]] inline Scalar integrateTrapezoidal(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    int n)
{
    if (n <= 0)
        return Scalar(0);

    const Scalar h = (b - a) / static_cast<Scalar>(n);
    Scalar sum = (f(a) + f(b)) * Scalar(0.5);

    for (int i = 1; i < n; ++i)
        sum += f(a + static_cast<Scalar>(i) * h);

    return sum * h;
}

[[nodiscard]] inline Scalar integrateSimpson(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    int n)
{
    if (n <= 0)
        return Scalar(0);

    if (n % 2 != 0)
        ++n;

    const Scalar h = (b - a) / static_cast<Scalar>(n);
    Scalar sum = f(a) + f(b);

    for (int i = 1; i < n; ++i)
    {
        const Scalar x = a + static_cast<Scalar>(i) * h;
        sum += f(x) * (i % 2 == 0 ? Scalar(2) : Scalar(4));
    }

    return sum * h / Scalar(3);
}

[[nodiscard]] inline Scalar derivativeCentral(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x + h) - f(x - h)) / (Scalar(2) * h);
}

[[nodiscard]] inline Scalar derivativeForward(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x + h) - f(x)) / h;
}

[[nodiscard]] inline Scalar derivativeBackward(
    std::function<Scalar(Scalar)> f,
    Scalar x,
    Scalar h = Scalar(1e-5))
{
    return (f(x) - f(x - h)) / h;
}

[[nodiscard]] inline mir4d::Result<Scalar> minimizeGoldenSection(
    std::function<Scalar(Scalar)> f,
    Scalar a,
    Scalar b,
    Scalar tol = Scalar(1e-7),
    int maxIter = 100)
{
    if (tol <= Scalar(0) || maxIter <= 0)
        return fail(mir4d::ErrorCode::InvalidArgument, "Некорректные tol/maxIter");

    if (b < a)
        std::swap(a, b);

    const Scalar resphi = Scalar(2) - (Scalar(1) + std::sqrt(Scalar(5))) / Scalar(2);

    Scalar x1 = a + resphi * (b - a);
    Scalar x2 = b - resphi * (b - a);
    Scalar f1 = f(x1);
    Scalar f2 = f(x2);

    for (int i = 0; i < maxIter; ++i)
    {
        if (std::abs(b - a) <= tol)
            return mir4d::success((a + b) * Scalar(0.5));

        if (f1 < f2)
        {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = a + resphi * (b - a);
            f1 = f(x1);
        }
        else
        {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = b - resphi * (b - a);
            f2 = f(x2);
        }
    }

    return fail(mir4d::ErrorCode::Internal,
        "Поиск золотым сечением не сошёлся за maxIter");
}

}
