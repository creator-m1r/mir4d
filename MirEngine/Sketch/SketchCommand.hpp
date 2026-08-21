#pragma once

#include "SketchConstraint.hpp"
#include "SketchDocument.hpp"
#include "SketchSnap.hpp"

#include <cstdint>
#include <optional>

namespace mir
{

class SketchCommandExecutor
{
public:
    std::uint32_t createLine(
        SketchDocument& document,
        SketchPoint2D start,
        SketchPoint2D end,
        bool construction = false) const
    {
        return document.geometry().addLine(start, end, construction);
    }

    std::optional<std::uint32_t> createLineWithSnap(
        SketchDocument& document,
        SketchPoint2D start,
        SketchPoint2D end,
        const SketchSnapCandidate* startSnap,
        const SketchSnapCandidate* endSnap) const
    {
        const auto id = createLine(document, start, end);

        if (startSnap && startSnap->geometryId != 0)
        {
            document.constraints().add(
                SketchConstraintType::Coincident,
                id,
                startSnap->geometryId);
        }

        if (endSnap && endSnap->geometryId != 0)
        {
            document.constraints().add(
                SketchConstraintType::Coincident,
                id,
                endSnap->geometryId);
        }

        return id;
    }
};

}
