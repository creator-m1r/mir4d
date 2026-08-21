
#pragma once

#include "Scalar.hpp"
#include <numbers>
#include <cmath>

namespace mir {

class Angle {
public:

    [[nodiscard]] static constexpr Angle radians(Scalar value) noexcept {
        return Angle(value, true);
    }

    [[nodiscard]] static constexpr Angle degrees(Scalar value) noexcept {

        constexpr Scalar DEG_TO_RAD = 3.14159265358979323846 / 180.0;
        return Angle(value * DEG_TO_RAD, true);
    }

    [[nodiscard]] constexpr Scalar radians() const noexcept {
        return m_radians;
    }

    [[nodiscard]] constexpr Scalar degrees() const noexcept {
        constexpr Scalar RAD_TO_DEG = 180.0 / 3.14159265358979323846;
        return m_radians * RAD_TO_DEG;
    }

    [[nodiscard]] static constexpr Angle zero() noexcept {
        return radians(0.0);
    }
    [[nodiscard]] static constexpr Angle halfPi() noexcept {
        return radians(1.5707963267948966);
    }
    [[nodiscard]] static constexpr Angle pi() noexcept {
        return radians(3.1415926535897932);
    }
    [[nodiscard]] static constexpr Angle twoPi() noexcept {
        return radians(6.2831853071795865);
    }

    friend constexpr Angle operator+(Angle a, Angle b) noexcept {
        return radians(a.m_radians + b.m_radians);
    }
    friend constexpr Angle operator-(Angle a, Angle b) noexcept {
        return radians(a.m_radians - b.m_radians);
    }
    friend constexpr Angle operator*(Angle a, Scalar s) noexcept {
        return radians(a.m_radians * s);
    }
    friend constexpr Angle operator/(Angle a, Scalar s) noexcept {
        return radians(a.m_radians / s);
    }
    friend constexpr Scalar operator/(Angle a, Angle b) noexcept {
        return a.m_radians / b.m_radians;
    }

    constexpr Angle operator-() const noexcept {
        return radians(-m_radians);
    }

    friend constexpr bool operator==(Angle a, Angle b) noexcept {
        return a.m_radians == b.m_radians;
    }
    friend constexpr bool operator!=(Angle a, Angle b) noexcept {
        return a.m_radians != b.m_radians;
    }

    [[nodiscard]] Angle normalized() const noexcept {
        Scalar twoPi = 2.0 * 3.14159265358979323846;
        Scalar normalized = std::fmod(m_radians, twoPi);
        if (normalized < 0.0) normalized += twoPi;
        return radians(normalized);
    }

private:

    constexpr Angle(Scalar radians, bool ) noexcept
        : m_radians(radians)
    {}

    Scalar m_radians = 0.0;
};

}