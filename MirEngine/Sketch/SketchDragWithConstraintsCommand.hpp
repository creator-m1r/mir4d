#pragma once

#include "SketchDragCommand.hpp"
#include "SketchConstraintInference.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace mir
{

class SketchDragWithConstraintsCommand final : public ISketchCommand
{
public:
    SketchDragWithConstraintsCommand(
        std::uint32_t geometryId,
        SketchDragHandle handle,
        SketchPoint2D original,
        SketchPoint2D final,
        std::vector<SketchInference> inferences)
        : geometryId_(geometryId),
          handle_(handle),
          original_(original),
          final_(final),
          inferences_(std::move(inferences))
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (!applyPoint(document, final_))
            return false;

        constraintIds_.clear();
        for (const auto& inference : inferences_)
        {
            const auto constraint = map(inference);
            if (!constraint)
                continue;

            const auto id = document.constraints().add(
                constraint->type,
                constraint->firstGeometry,
                constraint->secondGeometry,
                constraint->value,
                true);
            constraintIds_.push_back(id);
        }

        return true;
    }

    bool undo(SketchDocument& document) override
    {
        for (const auto id : constraintIds_)
            document.constraints().remove(id);

        constraintIds_.clear();
        return applyPoint(document, original_);
    }

    [[nodiscard]] const std::vector<std::uint32_t>& constraintIds() const noexcept
    {
        return constraintIds_;
    }

private:
    struct ConstraintData
    {
        SketchConstraintType type;
        std::uint32_t firstGeometry;
        std::uint32_t secondGeometry;
        double value;
    };

    static std::optional<ConstraintData> map(const SketchInference& inference)
    {
        switch (inference.type)
        {
        case SketchInferenceType::Horizontal:
            return ConstraintData{
                SketchConstraintType::Horizontal,
                inference.firstGeometryId,
                0,
                0.0};
        case SketchInferenceType::Vertical:
            return ConstraintData{
                SketchConstraintType::Vertical,
                inference.firstGeometryId,
                0,
                0.0};
        case SketchInferenceType::Coincident:
            return ConstraintData{
                SketchConstraintType::Coincident,
                inference.firstGeometryId,
                inference.secondGeometryId,
                0.0};
        case SketchInferenceType::Perpendicular:
            return ConstraintData{
                SketchConstraintType::Perpendicular,
                inference.firstGeometryId,
                inference.secondGeometryId,
                0.0};
        case SketchInferenceType::Midpoint:
        case SketchInferenceType::None:

            return std::nullopt;
        }

        return std::nullopt;
    }

    bool applyPoint(SketchDocument& document, SketchPoint2D point)
    {
        auto* item = document.geometry().findMutable(geometryId_);
        if (!item)
            return false;

        auto* line = std::get_if<SketchLine2D>(item);
        if (!line)
            return false;

        if (handle_ == SketchDragHandle::StartPoint)
            line->start = point;
        else
            line->end = point;

        return true;
    }

    std::uint32_t geometryId_{0};
    SketchDragHandle handle_{SketchDragHandle::EndPoint};
    SketchPoint2D original_{};
    SketchPoint2D final_{};
    std::vector<SketchInference> inferences_;
    std::vector<std::uint32_t> constraintIds_;
};

}
