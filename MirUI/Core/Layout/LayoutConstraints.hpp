// MirUI/Core/Layout/LayoutConstraints.hpp
// Size constraints for layout: minimum, maximum, preferred.
// Pure C++23, no platform dependencies.

#pragma once

#include "Size.hpp"

namespace MirUI {

struct LayoutConstraints {
    Size minimum   = { 0.0, 0.0 };
    Size maximum   = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
    Size preferred = { 0.0, 0.0 };

    constexpr LayoutConstraints() noexcept = default;
    constexpr LayoutConstraints(const Size& pref) noexcept
        : minimum(pref), maximum(pref), preferred(pref) {}
    constexpr LayoutConstraints(const Size& min, const Size& max, const Size& pref) noexcept
        : minimum(min), maximum(max), preferred(pref) {}

    // Convenience: create unconstrained (flexible) constraints.
    static constexpr LayoutConstraints flexible() noexcept {
        return LayoutConstraints(
            { 0.0, 0.0 },
            { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() },
            { 0.0, 0.0 }
        );
    }

    // Clamp a given size to the constraints.
    [[nodiscard]] constexpr Size clamp(const Size& size) const noexcept {
        return Size(
            std::clamp(size.width,  minimum.width,  maximum.width),
            std::clamp(size.height, minimum.height, maximum.height)
        );
    }

    friend constexpr bool operator==(const LayoutConstraints& a, const LayoutConstraints& b) noexcept {
        return a.minimum == b.minimum && a.maximum == b.maximum && a.preferred == b.preferred;
    }
    friend constexpr bool operator!=(const LayoutConstraints& a, const LayoutConstraints& b) noexcept {
        return !(a == b);
    }
};

} // namespace MirUI