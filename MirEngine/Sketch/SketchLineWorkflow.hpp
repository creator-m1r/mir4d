#pragma once

#include "SketchCommandHistory.hpp"
#include "SketchCompoundCommand.hpp"
#include "SketchInferenceEngine.hpp"

#include <memory>
#include <vector>

namespace mir
{

class SketchLineWorkflow
{
public:
    bool commit(
        SketchDocument& document,
        SketchCommandHistory& history,
        const SketchInferenceEngine& inference,
        SketchPoint2D start,
        SketchPoint2D end,
        bool construction = false) const
    {
        const auto lineId = previewLineId(document);
        const auto inferred = inference.inferLine(document.geometry(), lineId, start, end);

        auto command = std::make_unique<CreateLineWithInferenceCommand>(
            start,
            end,
            inferred,
            construction);

        return history.execute(std::move(command), document);
    }

private:
    [[nodiscard]] static std::uint32_t previewLineId(
        const SketchDocument& document) noexcept
    {

        return 0xFFFFFFFFu;
    }
};

}
