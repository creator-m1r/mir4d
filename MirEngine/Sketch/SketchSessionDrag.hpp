#pragma once

#include "SketchConstraintInference.hpp"
#include "SketchDragWithConstraintsCommand.hpp"
#include "SketchSession.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace mir
{

class SketchSessionDrag
{
public:
    explicit SketchSessionDrag(SketchSession& session,
                               double minimumConfidence = 0.85)
        : session_(session),
          inferenceGate_(minimumConfidence)
    {
    }

    [[nodiscard]] bool commit(
        std::uint32_t geometryId,
        SketchDragHandle handle,
        SketchPoint2D original,
        SketchPoint2D final,
        const std::vector<SketchInference>& previewInferences)
    {
        const SketchDragResolution preview{final, std::nullopt, previewInferences};
        const auto accepted = inferenceGate_.accepted(preview);

        auto command = std::make_unique<SketchDragWithConstraintsCommand>(
            geometryId,
            handle,
            original,
            final,
            accepted);

        const bool success = session_.history().execute(
            std::move(command),
            session_.document());

        if (success)
            session_.syncHistoryState();

        return success;
    }

private:
    SketchSession& session_;
    SketchConstraintInference inferenceGate_;
};

}
