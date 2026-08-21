#pragma once

#include "SketchDocument.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace mir
{

class ISketchCommand
{
public:
    virtual ~ISketchCommand() = default;
    virtual bool execute(SketchDocument& document) = 0;
    virtual bool undo(SketchDocument& document) = 0;
};

class CreateLineCommand final : public ISketchCommand
{
public:
    CreateLineCommand(SketchPoint2D start, SketchPoint2D end, bool construction = false)
        : start_(start), end_(end), construction_(construction)
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (createdId_ != 0)
            return true;

        createdId_ = document.geometry().addLine(start_, end_, construction_);
        return createdId_ != 0;
    }

    bool undo(SketchDocument& document) override
    {
        if (createdId_ == 0)
            return false;

        const bool removed = document.geometry().remove(createdId_);
        if (removed)
            createdId_ = 0;
        return removed;
    }

    [[nodiscard]] std::uint32_t createdId() const noexcept
    {
        return createdId_;
    }

private:
    SketchPoint2D start_{};
    SketchPoint2D end_{};
    bool construction_{false};
    std::uint32_t createdId_{0};
};

class SketchCommandHistory
{
public:
    explicit SketchCommandHistory(std::size_t maximumSize = 256)
        : maximumSize_(maximumSize)
    {
    }

    bool execute(
        std::unique_ptr<ISketchCommand> command,
        SketchDocument& document)
    {
        if (!command || !command->execute(document))
            return false;

        undoStack_.push_back(std::move(command));
        redoStack_.clear();
        trim();
        return true;
    }

    bool undo(SketchDocument& document)
    {
        if (undoStack_.empty())
            return false;

        auto command = std::move(undoStack_.back());
        undoStack_.pop_back();

        if (!command->undo(document))
        {
            undoStack_.push_back(std::move(command));
            return false;
        }

        redoStack_.push_back(std::move(command));
        return true;
    }

    bool redo(SketchDocument& document)
    {
        if (redoStack_.empty())
            return false;

        auto command = std::move(redoStack_.back());
        redoStack_.pop_back();

        if (!command->execute(document))
        {
            redoStack_.push_back(std::move(command));
            return false;
        }

        undoStack_.push_back(std::move(command));
        trim();
        return true;
    }

    [[nodiscard]] bool canUndo() const noexcept
    {
        return !undoStack_.empty();
    }

    [[nodiscard]] bool canRedo() const noexcept
    {
        return !redoStack_.empty();
    }

    void clear() noexcept
    {
        undoStack_.clear();
        redoStack_.clear();
    }

private:
    void trim()
    {
        if (maximumSize_ == 0)
        {
            undoStack_.clear();
            return;
        }

        if (undoStack_.size() > maximumSize_)
        {
            const auto count = undoStack_.size() - maximumSize_;
            undoStack_.erase(undoStack_.begin(), undoStack_.begin() + static_cast<std::ptrdiff_t>(count));
        }
    }

    std::size_t maximumSize_;
    std::vector<std::unique_ptr<ISketchCommand>> undoStack_;
    std::vector<std::unique_ptr<ISketchCommand>> redoStack_;
};

}
