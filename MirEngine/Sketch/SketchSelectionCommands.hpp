#pragma once

#include "SketchCommandHistory.hpp"
#include "SketchSelection.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

/// UI-independent selection operations for the active sketch session.
class SketchSelectionCommands
{
public:
    static void select(SketchSelection& selection, std::uint32_t geometryId, bool additive = false)
    {
        selection.select(geometryId, additive);
    }

    static void toggle(SketchSelection& selection, std::uint32_t geometryId)
    {
        selection.toggle(geometryId);
    }

    static void clear(SketchSelection& selection)
    {
        selection.clear();
    }
};

/// Reversible selection command for use by the command history when selection
/// changes need to participate in a larger atomic UI operation.
class SelectGeometryCommand final : public ISketchCommand
{
public:
    SelectGeometryCommand(SketchSelection& selection,
                          std::uint32_t geometryId,
                          bool additive = false)
        : selection_(selection), geometryId_(geometryId), additive_(additive)
    {
    }

    bool execute(SketchDocument&) override
    {
        previous_ = selection_.ids();
        selection_.select(geometryId_, additive_);
        return geometryId_ != 0;
    }

    bool undo(SketchDocument&) override
    {
        selection_.clear();
        for (const auto id : previous_)
            selection_.select(id, true);
        return true;
    }

private:
    SketchSelection& selection_;
    std::uint32_t geometryId_{0};
    bool additive_{false};
    std::vector<std::uint32_t> previous_;
};

class ClearSelectionCommand final : public ISketchCommand
{
public:
    explicit ClearSelectionCommand(SketchSelection& selection)
        : selection_(selection)
    {
    }

    bool execute(SketchDocument&) override
    {
        previous_ = selection_.ids();
        selection_.clear();
        return true;
    }

    bool undo(SketchDocument&) override
    {
        selection_.clear();
        for (const auto id : previous_)
            selection_.select(id, true);
        return true;
    }

private:
    SketchSelection& selection_;
    std::vector<std::uint32_t> previous_;
};

} // namespace mir
