// MirUI/Foundation/Animation/Animation.hpp
// Animation curve enumeration for MirUI.
// Pure C++23, no platform dependencies.

#pragma once

namespace MirUI {

enum class AnimationCurve {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    Spring
};

} // namespace MirUI