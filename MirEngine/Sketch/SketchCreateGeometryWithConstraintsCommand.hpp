#pragma once

#include "SketchCommandHistory.hpp"
#include "SketchConstraint.hpp"
#include "SketchInferenceEngine.hpp"

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace mir
{

/// Atomically creates one geometry entity and commits only high-confidence
/// inferred constraints that reference it.
class SketchCreateGeometryWithConstraintsCommand final : public ISketchCommand
{
public:
    SketchCreateGeometryWithConstraintsCommand(
        SketchGeometry geometry,
        std::vector<SketchInference> inferences,
        double minimumConfidence = 0.85)
        : geometry_(std::move(geometry)),
          inferences_(std::move(inferences)),
          minimumConfidence_(minimumConfidence)
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (inserted_)
            return true;

        geometryId_ = insertGeometry(document);
        if (geometryId_ == 0)
            return false;

        constraintIds_.clear();
        for (const auto& inference : inferences_)
        {
            if (inference.confidence < minimumConfidence_)
                continue;

            const auto type = toConstraintType(inference.type);
            if (!type.has_value())
                continue;

            const auto first = inference.firstGeometryId != 0
                ? inference.firstGeometryId
                : geometryId_;

            constraintIds_.push_back(
                document.constraints().add(*type, first, inference.secondGeometryId));
        }

        inserted_ = true;
        return true;
    }

    bool undo(SketchDocument& document) override
    {
        if (!inserted_)
            return false;

        for (auto it = constraintIds_.rbegin(); it != constraintIds_.rend(); ++it)
            document.constraints().remove(*it);

        if (!document.geometry().remove(geometryId_))
            return false;

        inserted_ = false;
        constraintIds_.clear();
        geometryId_ = 0;
        return true;
    }

private:
    [[nodiscard]] std::uint32_t insertGeometry(SketchDocument& document)
    {
        return std::visit(
            [&](const auto& value) -> std::uint32_t
            {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                    return document.geometry().addLine(value.start, value.end, value.construction);
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                    return document.geometry().addCircle(value.center, value.radius, value.construction);
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                    return document.geometry().addArc(
                        value.center,
                        value.radius,
                        value.startAngle,
                        value.endAngle,
                        value.construction);
                else
                    return 0;
            },
            geometry_);
    }

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

    SketchGeometry geometry_;
    std::vector<SketchInference> inferences_;
    std::vector<std::uint32_t> constraintIds_;
    double minimumConfidence_{0.85};
    std::uint32_t geometryId_{0};
    bool inserted_{false};
};

} // namespace mir
