#pragma once

#include "SketchCommandHistory.hpp"
#include "SketchInferenceEngine.hpp"
#include "SketchSelection.hpp"
#include "SketchSnap.hpp"

#include <cstddef>
#include <cstdint>

namespace mir
{

enum class SketchSolverStatus : std::uint8_t
{
    NotRun,
    Solved,
    UnderConstrained,
    OverConstrained,
    Failed
};

struct SketchSessionState
{
    SketchSolverStatus solverStatus{SketchSolverStatus::NotRun};
    int degreesOfFreedom{-1};
    bool canUndo{false};
    bool canRedo{false};
    std::uint64_t revision{0};
    std::size_t geometryCount{0};
    std::size_t constraintCount{0};
    std::size_t profileCount{0};
};

/// Runtime facade for one active sketch editing session.
/// It is the single model-facing object that the UI should observe.
class SketchSession
{
public:
    explicit SketchSession(std::size_t historySize = 256)
        : history_(historySize)
    {
    }

    [[nodiscard]] SketchDocument& document() noexcept { return document_; }
    [[nodiscard]] const SketchDocument& document() const noexcept { return document_; }

    [[nodiscard]] SketchCommandHistory& history() noexcept { return history_; }
    [[nodiscard]] const SketchCommandHistory& history() const noexcept { return history_; }

    [[nodiscard]] SketchSnapEngine& snap() noexcept { return snap_; }
    [[nodiscard]] const SketchSnapEngine& snap() const noexcept { return snap_; }

    [[nodiscard]] SketchInferenceEngine& inference() noexcept { return inference_; }
    [[nodiscard]] const SketchInferenceEngine& inference() const noexcept { return inference_; }

    [[nodiscard]] SketchSelection& selection() noexcept { return selection_; }
    [[nodiscard]] const SketchSelection& selection() const noexcept { return selection_; }

    [[nodiscard]] const SketchSessionState& state() const noexcept { return state_; }

    void setSolverState(SketchSolverStatus status, int degreesOfFreedom) noexcept
    {
        state_.solverStatus = status;
        state_.degreesOfFreedom = degreesOfFreedom;
        syncHistoryState();
    }

    /// Bumps the document revision and refreshes derived counts.
    /// Call after every committed mutation so the UI can react.
    void touch() noexcept
    {
        state_.revision += 1;
        state_.geometryCount = document_.geometry().all().size();
        state_.constraintCount = document_.constraints().all().size();
        syncHistoryState();
    }

    void setProfileCount(std::size_t count) noexcept
    {
        state_.profileCount = count;
        state_.revision += 1;
    }

    [[nodiscard]] std::uint64_t revision() const noexcept { return state_.revision; }

    void syncHistoryState() noexcept
    {
        state_.canUndo = history_.canUndo();
        state_.canRedo = history_.canRedo();
    }

    bool undo()
    {
        const bool changed = history_.undo(document_);
        if (changed)
            syncHistoryState();
        return changed;
    }

    bool redo()
    {
        const bool changed = history_.redo(document_);
        if (changed)
            syncHistoryState();
        return changed;
    }

private:
    SketchDocument document_;
    SketchCommandHistory history_;
    SketchSnapEngine snap_;
    SketchInferenceEngine inference_;
    SketchSelection selection_;
    SketchSessionState state_;
};

} // namespace mir
