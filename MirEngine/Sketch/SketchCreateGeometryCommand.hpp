#pragma once

#include "SketchCommandHistory.hpp"

#include <cstdint>
#include <utility>

namespace mir
{

/// Adds one complete sketch entity as a single undoable operation.
class SketchCreateGeometryCommand final : public ISketchCommand
{
public:
    explicit SketchCreateGeometryCommand(SketchGeometry geometry)
        : geometry_(std::move(geometry))
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (inserted_)
            return true;

        document.geometry().add(geometry_);
        inserted_ = true;
        return true;
    }

    bool undo(SketchDocument& document) override
    {
        if (!inserted_)
            return false;

        const auto id = std::visit(
            [](const auto& item) { return item.id; },
            geometry_);

        const bool removed = document.geometry().remove(id);
        if (removed)
            inserted_ = false;
        return removed;
    }

    [[nodiscard]] std::uint32_t insertedId() const noexcept
    {
        if (!inserted_)
            return 0;
        return std::visit(
            [](const auto& item) { return item.id; },
            geometry_);
    }

private:
    SketchGeometry geometry_;
    bool inserted_{false};
};

} // namespace mir
