#pragma once

#include "SketchDragCommand.hpp"
#include "SketchInferenceEngine.hpp"
#include "SketchSnap.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

struct SketchDragResolution
{
    SketchPoint2D point{};
    std::optional<SketchSnapCandidate> snap;
    std::vector<SketchInference> inferences;
};

class SketchDragResolver
{
public:
    SketchDragResolver(
        double snapTolerance = 8.0,
        double angularTolerance = 0.08,
        double pointTolerance = 8.0)
        : snap_(snapTolerance),
          inference_(angularTolerance, pointTolerance)
    {
    }

    [[nodiscard]] SketchDragResolution resolve(
        const SketchGeometryStore& geometry,
        std::uint32_t lineId,
        SketchDragHandle handle,
        SketchPoint2D original,
        SketchPoint2D cursor) const noexcept
    {
        SketchDragResolution result;
        result.point = cursor;

        if (const auto candidate = snap_.nearest(geometry, cursor))
        {

            if (candidate->geometryId != lineId)
            {
                result.point = candidate->point;
                result.snap = candidate;
            }
        }

        const auto item = geometry.find(lineId);
        if (!item)
            return result;

        const auto* line = std::get_if<SketchLine2D>(&*item);
        if (!line)
            return result;

        const SketchPoint2D start =
            handle == SketchDragHandle::StartPoint ? result.point : line->start;
        const SketchPoint2D end =
            handle == SketchDragHandle::EndPoint ? result.point : line->end;

        result.inferences = inference_.inferLine(
            geometry,
            lineId,
            start,
            end);

        if (result.snap && result.snap->type == SketchSnapType::Endpoint)
        {
            result.inferences.erase(
                std::remove_if(
                    result.inferences.begin(),
                    result.inferences.end(),
                    [&](const SketchInference& inference)
                    {
                        return inference.type == SketchInferenceType::Coincident &&
                               inference.secondGeometryId == result.snap->geometryId;
                    }),
                result.inferences.end());
        }

        (void)original;
        return result;
    }

private:
    SketchSnapEngine snap_;
    SketchInferenceEngine inference_;
};

}
