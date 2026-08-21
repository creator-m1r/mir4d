#pragma once

#include "SketchCommandHistory.hpp"
#include "SketchInferenceEngine.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace mir
{

class CreateLineWithInferenceCommand final : public ISketchCommand
{
public:
    CreateLineWithInferenceCommand(
        SketchPoint2D start,
        SketchPoint2D end,
        std::vector<SketchInference> inferences,
        bool construction = false)
        : start_(start),
          end_(end),
          inferences_(std::move(inferences)),
          construction_(construction)
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (lineId_ != 0)
            return true;

        lineId_ = document.geometry().addLine(start_, end_, construction_);
        if (lineId_ == 0)
            return false;

        createdConstraintIds_.clear();

        for (const auto& inference : inferences_)
        {
            const auto type = toConstraintType(inference.type);
            if (!type)
                continue;

            const auto id = document.constraints().add(
                *type,
                lineId_,
                inference.secondGeometryId,
                0.0,
                true);
            createdConstraintIds_.push_back(id);
        }

        return true;
    }

    bool undo(SketchDocument& document) override
    {
        bool success = true;

        for (auto it = createdConstraintIds_.rbegin();
             it != createdConstraintIds_.rend();
             ++it)
        {
            success = document.constraints().remove(*it) && success;
        }

        if (lineId_ != 0)
        {
            success = document.geometry().remove(lineId_) && success;
            if (success)
                lineId_ = 0;
        }

        if (success)
            createdConstraintIds_.clear();

        return success;
    }

    [[nodiscard]] std::uint32_t lineId() const noexcept
    {
        return lineId_;
    }

    [[nodiscard]] const std::vector<std::uint32_t>& createdConstraintIds() const noexcept
    {
        return createdConstraintIds_;
    }

private:
    [[nodiscard]] static std::optional<SketchConstraintType> toConstraintType(
        SketchInferenceType type) noexcept
    {
        switch (type)
        {
        case SketchInferenceType::Horizontal:
            return SketchConstraintType::Horizontal;
        case SketchInferenceType::Vertical:
            return SketchConstraintType::Vertical;
        case SketchInferenceType::Coincident:
            return SketchConstraintType::Coincident;
        case SketchInferenceType::Perpendicular:
            return SketchConstraintType::Perpendicular;
        case SketchInferenceType::Midpoint:
        case SketchInferenceType::None:
            return std::nullopt;
        }

        return std::nullopt;
    }

    SketchPoint2D start_{};
    SketchPoint2D end_{};
    std::vector<SketchInference> inferences_;
    bool construction_{false};
    std::uint32_t lineId_{0};
    std::vector<std::uint32_t> createdConstraintIds_;
};

}
