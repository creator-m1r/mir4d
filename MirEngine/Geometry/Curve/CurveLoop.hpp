#pragma once

#include "Curve.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace mir
{

class CurveLoop3
{
public:
    using CurvePtr = std::shared_ptr<const Curve3>;

    CurveLoop3() = default;
    explicit CurveLoop3(std::vector<CurvePtr> curves, Scalar tolerance = 1e-9)
        : curves_(std::move(curves)), tolerance_(std::max(Scalar(0.0), tolerance)) {}

    [[nodiscard]] const std::vector<CurvePtr>& curves() const noexcept { return curves_; }
    [[nodiscard]] std::size_t size() const noexcept { return curves_.size(); }
    [[nodiscard]] bool empty() const noexcept { return curves_.empty(); }
    [[nodiscard]] Scalar tolerance() const noexcept { return tolerance_; }

    void clear() noexcept { curves_.clear(); }
    void add(CurvePtr curve) { if (curve) curves_.push_back(std::move(curve)); }

    [[nodiscard]] Point3 startPoint() const noexcept
    {
        if (curves_.empty() || !curves_.front()) return Point3::origin();
        const auto& curve = curves_.front();
        return curve->pointAt(curve->parameterStart());
    }

    [[nodiscard]] Point3 endPoint() const noexcept
    {
        if (curves_.empty() || !curves_.back()) return Point3::origin();
        const auto& curve = curves_.back();
        return curve->pointAt(curve->parameterEnd());
    }

    [[nodiscard]] bool isClosed() const noexcept
    {
        if (curves_.empty()) return false;
        for (std::size_t i = 0; i < curves_.size(); ++i)
        {
            const auto& current = curves_[i];
            const auto& next = curves_[(i + 1) % curves_.size()];
            if (!current || !next) return false;
            if (current->pointAt(current->parameterEnd()).distance(next->pointAt(next->parameterStart())) > tolerance_)
                return false;
        }
        return true;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (curves_.size() < 2) return false;
        for (const auto& curve : curves_)
        {
            if (!curve) return false;
            const Scalar a = curve->parameterStart();
            const Scalar b = curve->parameterEnd();
            if (!std::isfinite(a) || !std::isfinite(b) || b < a) return false;
        }
        return isClosed();
    }

    [[nodiscard]] Scalar length() const noexcept
    {
        Scalar result = 0.0;
        for (const auto& curve : curves_) if (curve) result += curve->length();
        return result;
    }

    [[nodiscard]] Point3 boundsMin() const noexcept
    {
        if (curves_.empty() || !curves_.front()) return Point3::origin();
        Point3 result = curves_.front()->boundsMin();
        for (std::size_t i = 1; i < curves_.size(); ++i)
        {
            if (!curves_[i]) continue;
            const Point3 p = curves_[i]->boundsMin();
            result.x = std::min(result.x, p.x); result.y = std::min(result.y, p.y); result.z = std::min(result.z, p.z);
        }
        return result;
    }

    [[nodiscard]] Point3 boundsMax() const noexcept
    {
        if (curves_.empty() || !curves_.front()) return Point3::origin();
        Point3 result = curves_.front()->boundsMax();
        for (std::size_t i = 1; i < curves_.size(); ++i)
        {
            if (!curves_[i]) continue;
            const Point3 p = curves_[i]->boundsMax();
            result.x = std::max(result.x, p.x); result.y = std::max(result.y, p.y); result.z = std::max(result.z, p.z);
        }
        return result;
    }

private:
    std::vector<CurvePtr> curves_;
    Scalar tolerance_{1e-9};
};

}
