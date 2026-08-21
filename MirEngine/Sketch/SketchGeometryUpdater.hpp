#pragma once

#include "SketchGeometry.hpp"
#include "SketchSolverState.hpp"

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <variant>

namespace mir
{

class SketchGeometryUpdater
{
public:
    [[nodiscard]] bool apply(
        SketchGeometryStore& geometry,
        const SketchSolverState& state) const noexcept
    {
        bool changed = false;

        for (auto& item : geometry.mutableAllForSolver())
        {
            std::visit([&](auto& value)
            {
                const auto* range = state.binding().find(value.id);
                if (!range || range->offset + range->count > state.size())
                    return;

                const auto& v = state.variables();
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    if (range->count >= 4)
                    {
                        value.start.x = v[range->offset];
                        value.start.y = v[range->offset + 1];
                        value.end.x = v[range->offset + 2];
                        value.end.y = v[range->offset + 3];
                        changed = true;
                    }
                }
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                {
                    if (range->count >= 3)
                    {
                        value.center.x = v[range->offset];
                        value.center.y = v[range->offset + 1];
                        value.radius = std::max(0.0, v[range->offset + 2]);
                        changed = true;
                    }
                }
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                {
                    if (range->count >= 5)
                    {
                        value.center.x = v[range->offset];
                        value.center.y = v[range->offset + 1];
                        value.radius = std::max(0.0, v[range->offset + 2]);
                        value.startAngle = v[range->offset + 3];
                        value.endAngle = v[range->offset + 4];
                        changed = true;
                    }
                }
            }, item);
        }

        return changed;
    }
};

}
