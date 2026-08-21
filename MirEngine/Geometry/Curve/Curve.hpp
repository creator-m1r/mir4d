#pragma once

#include "../../Core/Types/Scalar.hpp"
#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <cmath>
#include <limits>

namespace mir
{

/// Common geometric contract for three-dimensional parametric curves.
class Curve3
{
public:
    virtual ~Curve3() = default;

    [[nodiscard]] virtual Point3 pointAt(Scalar t) const = 0;
    [[nodiscard]] virtual Vector3 tangentAt(Scalar t) const = 0;
    [[nodiscard]] virtual Scalar parameterStart() const noexcept = 0;
    [[nodiscard]] virtual Scalar parameterEnd() const noexcept = 0;

    [[nodiscard]] Scalar parameterLength() const noexcept
    {
        return parameterEnd() - parameterStart();
    }

    [[nodiscard]] virtual Scalar length() const noexcept = 0;
    [[nodiscard]] virtual Point3 closestPoint(const Point3& point) const noexcept = 0;
    [[nodiscard]] virtual bool contains(const Point3& point, Scalar tolerance = Scalar(1e-9)) const noexcept = 0;
    [[nodiscard]] virtual Point3 boundsMin() const noexcept = 0;
    [[nodiscard]] virtual Point3 boundsMax() const noexcept = 0;

    [[nodiscard]] bool isValidParameter(Scalar t) const noexcept
    {
        return std::isfinite(t) && t >= parameterStart() && t <= parameterEnd();
    }
};

} // namespace mir
