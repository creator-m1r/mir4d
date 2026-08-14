#pragma once

#include "SketchDragResolver.hpp"
#include "SketchConstraintInference.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

struct SketchDragPreviewState
{
    SketchDragResolution resolution{};
    std::vector<SketchInference> acceptedInferences;
    bool hasSnap{false};

    void update(const SketchDragResolution& value,
                const SketchConstraintInference& gate)
    {
        resolution = value;
        acceptedInferences = gate.accepted(value);
        hasSnap = value.snap.has_value();
    }

    void clear() noexcept
    {
        resolution = {};
        acceptedInferences.clear();
        hasSnap = false;
    }
};

} // namespace mir
