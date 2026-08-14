#pragma once

#include "SketchCommandHistory.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>

namespace mir
{

enum class SketchLineParameter : std::uint8_t
{
    StartX,
    StartY,
    EndX,
    EndY,
    Length,
    AngleRadians
};

/// Changes one line parameter while keeping the operation reversible.
/// Length and angle are applied around the line start point.
class SetLineParameterCommand final : public ISketchCommand
{
public:
    SetLineParameterCommand(
        std::uint32_t geometryId,
        SketchLineParameter parameter,
        double value)
        : geometryId_(geometryId), parameter_(parameter), requestedValue_(value)
    {
    }

    bool execute(SketchDocument& document) override
    {
        const auto item = document.geometry().find(geometryId_);
        if (!item)
            return false;

        const auto* line = std::get_if<SketchLine2D>(&*item);
        if (!line)
            return false;

        if (!captured_)
        {
            oldStart_ = line->start;
            oldEnd_ = line->end;
            captured_ = true;
        }

        return apply(document, requestedValue_);
    }

    bool undo(SketchDocument& document) override
    {
        if (!captured_)
            return false;

        return restore(document);
    }

private:
    bool apply(SketchDocument& document, double value)
    {
        auto* item = document.geometry().findMutable(geometryId_);
        if (!item)
            return false;

        auto* line = std::get_if<SketchLine2D>(item);
        if (!line)
            return false;

        switch (parameter_)
        {
        case SketchLineParameter::StartX:
            line->start.x = value;
            break;
        case SketchLineParameter::StartY:
            line->start.y = value;
            break;
        case SketchLineParameter::EndX:
            line->end.x = value;
            break;
        case SketchLineParameter::EndY:
            line->end.y = value;
            break;
        case SketchLineParameter::Length:
        {
            const double angle = std::atan2(
                line->end.y - line->start.y,
                line->end.x - line->start.x);
            line->end.x = line->start.x + value * std::cos(angle);
            line->end.y = line->start.y + value * std::sin(angle);
            break;
        }
        case SketchLineParameter::AngleRadians:
        {
            const double dx = line->end.x - line->start.x;
            const double dy = line->end.y - line->start.y;
            const double length = std::sqrt(dx * dx + dy * dy);
            if (length <= 1e-12)
                return false;
            line->end.x = line->start.x + length * std::cos(value);
            line->end.y = line->start.y + length * std::sin(value);
            break;
        }
        }

        return true;
    }

    bool restore(SketchDocument& document)
    {
        auto* item = document.geometry().findMutable(geometryId_);
        if (!item)
            return false;

        auto* line = std::get_if<SketchLine2D>(item);
        if (!line)
            return false;

        line->start = oldStart_;
        line->end = oldEnd_;
        return true;
    }

    std::uint32_t geometryId_{0};
    SketchLineParameter parameter_{};
    double requestedValue_{0.0};
    SketchPoint2D oldStart_{};
    SketchPoint2D oldEnd_{};
    bool captured_{false};
};

} // namespace mir
