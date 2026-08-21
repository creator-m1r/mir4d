
#pragma once

#include "../../Core/Result.hpp"
#include "../../Core/Types/Scalar.hpp"
#include "../Vector/Vector.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
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

[[nodiscard]] inline Scalar evaluatePolynomial(
    const std::vector<Scalar>& coeffs,
    Scalar x)
{
    Scalar result = Scalar(0);
    for (auto it = coeffs.rbegin(); it != coeffs.rend(); ++it)
        result = result * x + (*it);
    return result;
}

[[nodiscard]] inline std::vector<Scalar> derivativePolynomial(
    const std::vector<Scalar>& coeffs)
{
    if (coeffs.size() <= 1)
        return {};

    std::vector<Scalar> derivative(coeffs.size() - 1);
    for (std::size_t i = 1; i < coeffs.size(); ++i)
        derivative[i - 1] = coeffs[i] * static_cast<Scalar>(i);
    return derivative;
}

[[nodiscard]] inline mir4d::Result<Scalar> lagrangeInterpolate(
    const std::vector<std::pair<Scalar, Scalar>>& points,
    Scalar x)
{
    if (points.size() < 2)
        return fail(mir4d::ErrorCode::InvalidArgument, "Нужно не менее двух узлов");

    Scalar result = Scalar(0);

    for (std::size_t i = 0; i < points.size(); ++i)
    {
        Scalar term = points[i].second;

        for (std::size_t j = 0; j < points.size(); ++j)
        {
            if (i == j)
                continue;

            const Scalar denom = points[i].first - points[j].first;
            if (std::abs(denom) <= Scalar(1e-14))
                return fail(mir4d::ErrorCode::InvalidArgument,
                    "Дублирующиеся узлы интерполяции");

            term *= (x - points[j].first) / denom;
        }

        result += term;
    }

    return mir4d::success(result);
}

[[nodiscard]] inline mir4d::Result<Scalar> linearInterpolate(
    const std::vector<std::pair<Scalar, Scalar>>& points,
    Scalar x)
{
    if (points.size() < 2)
        return fail(mir4d::ErrorCode::InvalidArgument, "Нужно не менее двух узлов");

    for (std::size_t i = 0; i + 1 < points.size(); ++i)
    {
        const Scalar x0 = points[i].first;
        const Scalar x1 = points[i + 1].first;

        if ((x >= x0 && x <= x1) || (x >= x1 && x <= x0))
        {
            const Scalar y0 = points[i].second;
            const Scalar y1 = points[i + 1].second;
            const Scalar denom = x1 - x0;
            if (std::abs(denom) <= Scalar(1e-14))
                return mir4d::success(y0);
            const Scalar t = (x - x0) / denom;
            return mir4d::success(y0 + t * (y1 - y0));
        }
    }

    const std::pair<Scalar, Scalar>& a = points.front();
    const std::pair<Scalar, Scalar>& b = points.back();
    const Scalar denom = b.first - a.first;
    if (std::abs(denom) <= Scalar(1e-14))
        return mir4d::success(a.second);
    const Scalar t = (x - a.first) / denom;
    return mir4d::success(a.second + t * (b.second - a.second));
}

[[nodiscard]] inline Vector3 cubicBezier(
    const Vector3& p0,
    const Vector3& p1,
    const Vector3& p2,
    const Vector3& p3,
    Scalar t)
{
    const Scalar u = Scalar(1) - t;
    const Scalar uu = u * u;
    const Scalar tt = t * t;
    const Scalar a = uu * u;
    const Scalar b = Scalar(3) * uu * t;
    const Scalar c = Scalar(3) * u * tt;
    const Scalar d = tt * t;

    return {
        a * p0.x + b * p1.x + c * p2.x + d * p3.x,
        a * p0.y + b * p1.y + c * p2.y + d * p3.y,
        a * p0.z + b * p1.z + c * p2.z + d * p3.z};
}

[[nodiscard]] inline mir4d::Result<Vector3> bezierPoint(
    const std::vector<Vector3>& control,
    Scalar t)
{
    if (control.empty())
        return fail(mir4d::ErrorCode::InvalidArgument, "Нет опорных точек Безье");

    std::vector<Vector3> pts = control;

    while (pts.size() > 1)
    {
        std::vector<Vector3> next(pts.size() - 1);
        for (std::size_t i = 0; i + 1 < pts.size(); ++i)
        {
            next[i] = {
                pts[i].x + t * (pts[i + 1].x - pts[i].x),
                pts[i].y + t * (pts[i + 1].y - pts[i].y),
                pts[i].z + t * (pts[i + 1].z - pts[i].z)};
        }
        pts = std::move(next);
    }

    return mir4d::success(pts.front());
}

[[nodiscard]] inline Vector3 catmullRom(
    const Vector3& p0,
    const Vector3& p1,
    const Vector3& p2,
    const Vector3& p3,
    Scalar t)
{
    const Scalar t2 = t * t;
    const Scalar t3 = t2 * t;

    auto comp = [&](Scalar a, Scalar b, Scalar c, Scalar d) -> Scalar
    {
        return Scalar(0.5) * (
            (Scalar(2) * b) +
            (-a + c) * t +
            (Scalar(2) * a - Scalar(5) * b + Scalar(4) * c - d) * t2 +
            (-a + Scalar(3) * b - Scalar(3) * c + d) * t3);
    };

    return {
        comp(p0.x, p1.x, p2.x, p3.x),
        comp(p0.y, p1.y, p2.y, p3.y),
        comp(p0.z, p1.z, p2.z, p3.z)};
}

[[nodiscard]] inline Vector3 cubicHermite(
    const Vector3& p0,
    const Vector3& p1,
    const Vector3& m0,
    const Vector3& m1,
    Scalar t)
{
    const Scalar t2 = t * t;
    const Scalar t3 = t2 * t;
    const Scalar h00 = Scalar(2) * t3 - Scalar(3) * t2 + Scalar(1);
    const Scalar h10 = t3 - Scalar(2) * t2 + t;
    const Scalar h01 = -Scalar(2) * t3 + Scalar(3) * t2;
    const Scalar h11 = t3 - t2;

    return {
        h00 * p0.x + h10 * m0.x + h01 * p1.x + h11 * m1.x,
        h00 * p0.y + h10 * m0.y + h01 * p1.y + h11 * m1.y,
        h00 * p0.z + h10 * m0.z + h01 * p1.z + h11 * m1.z};
}

}
