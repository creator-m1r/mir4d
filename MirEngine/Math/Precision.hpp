
#pragma once

#include "../Core/Types/Scalar.hpp"
#include "Vector/Vector3.hpp"
#include <cmath>

namespace mir {

class Precision {
public:

    Scalar linearTolerance  = Scalar(1e-10);
    Scalar angularTolerance = Scalar(1e-12);

    constexpr Precision() noexcept = default;

    constexpr Precision(Scalar linear, Scalar angular) noexcept
        : linearTolerance(linear), angularTolerance(angular)
    {}

    [[nodiscard]] bool areEqual(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= linearTolerance;
    }

    [[nodiscard]] bool isZero(Scalar value) const noexcept {
        return std::abs(value) <= linearTolerance;
    }

    [[nodiscard]] bool areEqual(const Vector3& a, const Vector3& b) const noexcept {
        return areEqual(a.x, b.x) && areEqual(a.y, b.y) && areEqual(a.z, b.z);
    }

    [[nodiscard]] bool isZero(const Vector3& v) const noexcept {
        return isZero(v.x) && isZero(v.y) && isZero(v.z);
    }

    [[nodiscard]] bool areEqualAngles(Scalar a, Scalar b) const noexcept {
        return std::abs(a - b) <= angularTolerance;
    }

    [[nodiscard]] bool isZeroAngle(Scalar angle) const noexcept {
        return std::abs(angle) <= angularTolerance;
    }

    static bool approximatelyEqual(Scalar a, Scalar b) noexcept {
        static Precision defaultPrecision;
        return defaultPrecision.areEqual(a, b);
    }

    static bool approximatelyZero(Scalar value) noexcept {
        static Precision defaultPrecision;
        return defaultPrecision.isZero(value);
    }

    static Precision& global() noexcept {
        static Precision s_global;
        return s_global;
    }
};

}