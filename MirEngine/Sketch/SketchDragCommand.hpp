#pragma once

#include "SketchCommandHistory.hpp"

#include <cstdint>
#include <variant>

namespace mir
{

enum class SketchDragHandle : std::uint8_t
{
    StartPoint,
    EndPoint
};

class SketchDragLineCommand final : public ISketchCommand
{
public:
    SketchDragLineCommand(
        std::uint32_t geometryId,
        SketchDragHandle handle,
        SketchPoint2D original,
        SketchPoint2D final)
        : geometryId_(geometryId),
          handle_(handle),
          original_(original),
          final_(final)
    {
    }

    bool execute(SketchDocument& document) override
    {
        return apply(document, final_);
    }

    bool undo(SketchDocument& document) override
    {
        return apply(document, original_);
    }

private:
    bool apply(SketchDocument& document, SketchPoint2D point)
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
};

}
