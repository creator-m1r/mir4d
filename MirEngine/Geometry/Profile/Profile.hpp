#pragma once

#include "../Curve/CurveLoop.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mir
{

/// A planar CAD profile consisting of one closed outer loop and zero or more closed inner loops.
class Profile3
{
public:
    Profile3() = default;

    explicit Profile3(CurveLoop3 outer,
                      std::vector<CurveLoop3> innerLoops = {},
                      Scalar tolerance = Scalar(1e-9))
        : outer_(std::move(outer))
        , innerLoops_(std::move(innerLoops))
        , tolerance_(std::max(Scalar(0.0), tolerance))
    {
    }

    [[nodiscard]] const CurveLoop3& outer() const noexcept { return outer_; }
    [[nodiscard]] const std::vector<CurveLoop3>& innerLoops() const noexcept { return innerLoops_; }
    [[nodiscard]] Scalar tolerance() const noexcept { return tolerance_; }

    void setOuter(CurveLoop3 outer) noexcept { outer_ = std::move(outer); }
    void addInner(CurveLoop3 inner) { innerLoops_.push_back(std::move(inner)); }

    [[nodiscard]] bool isPlanar(Scalar tolerance = Scalar(-1.0)) const noexcept
    {
        const Scalar eps = tolerance >= Scalar(0.0) ? tolerance : tolerance_;
        return isLoopPlanar(outer_, eps) &&
               std::all_of(innerLoops_.begin(), innerLoops_.end(),
                   [eps](const CurveLoop3& loop) { return isLoopPlanar(loop, eps); });
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!outer_.isValid()) return false;
        for (const auto& loop : innerLoops_)
            if (!loop.isValid()) return false;
        return isPlanar();
    }

    [[nodiscard]] Point3 boundsMin() const noexcept
    {
        Point3 result = outer_.boundsMin();
        for (const auto& loop : innerLoops_)
        {
            const Point3 p = loop.boundsMin();
            result.x = std::min(result.x, p.x);
            result.y = std::min(result.y, p.y);
            result.z = std::min(result.z, p.z);
        }
        return result;
    }

    [[nodiscard]] Point3 boundsMax() const noexcept
    {
        Point3 result = outer_.boundsMax();
        for (const auto& loop : innerLoops_)
        {
            const Point3 p = loop.boundsMax();
            result.x = std::max(result.x, p.x);
            result.y = std::max(result.y, p.y);
            result.z = std::max(result.z, p.z);
        }
        return result;
    }

private:
    static bool isLoopPlanar(const CurveLoop3& loop, Scalar tolerance) noexcept
    {
        if (!loop.isValid()) return false;
        const auto& curves = loop.curves();
        if (curves.empty()) return false;

        const Point3 p0 = curves.front()->pointAt(curves.front()->parameterStart());
        Vector3 firstDirection{};

        for (const auto& curve : curves)
        {
            if (!curve) return false;
            const Scalar a = curve->parameterStart();
            const Scalar b = curve->parameterEnd();
            const Scalar mid = a + (b - a) * Scalar(0.5);
            const Point3 samples[] = {curve->pointAt(a), curve->pointAt(mid), curve->pointAt(b)};

            for (const Point3& p : samples)
            {
                const Vector3 v = p - p0;
                if (v.lengthSquared() <= tolerance * tolerance) continue;
                if (firstDirection.isZero()) { firstDirection = v; continue; }

                const Vector3 candidateNormal = firstDirection.cross(v);
                if (candidateNormal.lengthSquared() > tolerance * tolerance)
                {
                    const Vector3 unitNormal = candidateNormal.normalized();
                    for (const auto& check : curves)
                    {
                        if (!check) return false;
                        const Scalar ca = check->parameterStart();
                        const Scalar cb = check->parameterEnd();
                        const Scalar cm = ca + (cb - ca) * Scalar(0.5);
                        const Point3 checkSamples[] = {check->pointAt(ca), check->pointAt(cm), check->pointAt(cb)};
                        for (const Point3& q : checkSamples)
                            if (std::abs((q - p0).dot(unitNormal)) > tolerance) return false;
                    }
                    return true;
                }
            }
        }
        return false;
    }

    CurveLoop3 outer_;
    std::vector<CurveLoop3> innerLoops_;
    Scalar tolerance_{1e-9};
};

} // namespace mir
