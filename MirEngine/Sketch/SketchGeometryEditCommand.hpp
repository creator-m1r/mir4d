#pragma once

#include "SketchCommandHistory.hpp"

#include <cstdint>
#include <variant>

namespace mir
{

enum class SketchGeometryHandle : std::uint8_t
{
    LineStart,
    LineEnd,
    CircleCenter,
    CircleRadius,
    ArcCenter,
    ArcStart,
    ArcEnd
};

/// Generic reversible edit for the currently supported sketch geometry.
/// A future implementation can use the same command shape for splines,
/// ellipses and other non-primitive sketch entities.
class SketchGeometryEditCommand final : public ISketchCommand
{
public:
    SketchGeometryEditCommand(
        std::uint32_t geometryId,
        SketchGeometry before,
        SketchGeometry after)
        : geometryId_(geometryId), before_(std::move(before)), after_(std::move(after))
    {
    }

    bool execute(SketchDocument& document) override
    {
        return replace(document, after_);
    }

    bool undo(SketchDocument& document) override
    {
        return replace(document, before_);
    }

private:
    bool replace(SketchDocument& document, const SketchGeometry& value)
    {
        auto& geometries = document.geometry().mutableAllForSolver();
        for (auto& geometry : geometries)
        {
            const auto id = std::visit(
                [](const auto& item) { return item.id; },
                geometry);

            if (id == geometryId_)
            {
                geometry = value;
                return true;
            }
        }
        return false;
    }

    std::uint32_t geometryId_{0};
    SketchGeometry before_;
    SketchGeometry after_;
};

} // namespace mir
