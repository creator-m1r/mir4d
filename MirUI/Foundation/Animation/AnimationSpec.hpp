
#pragma once

#include "Animation.hpp"

namespace MirUI {

struct AnimationSpec {
    double duration = 0.25;
    double delay    = 0.0;
    AnimationCurve curve = AnimationCurve::EaseInOut;

    double damping  = 0.6;
    double response = 0.4;

    static constexpr AnimationSpec immediate() { return {0.0, 0.0, AnimationCurve::Linear}; }
    static constexpr AnimationSpec fast()       { return {0.15, 0.0, AnimationCurve::EaseOut}; }
    static constexpr AnimationSpec smooth()     { return {0.3, 0.0, AnimationCurve::EaseInOut}; }
    static constexpr AnimationSpec spring()     {
        return {0.4, 0.0, AnimationCurve::Spring, 0.55, 0.5};
    }
};

}