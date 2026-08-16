// MirEngine/Document/SceneCommandHistory.hpp
// Undoable scene mutation commands (execute / undo / redo).
//
// MirEngine Scene is the single source of truth; commands mutate only the
// scene through the canonical API and the renderer observes the changes.
// This history extends the existing document Command model (CommandType::Move
// / CommandType::Delete) with executable implementations; it is not a second
// command system.

#pragma once

#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace mir4d
{

/// Undoable scene mutation command.
class SceneCommand
{
public:
    virtual ~SceneCommand() = default;

    virtual void execute(mir::Scene& scene) = 0;
    virtual void undo(mir::Scene& scene) = 0;

    [[nodiscard]] virtual const char* description() const noexcept = 0;
};

/// Moves one object from a captured start transform to a target transform.
/// The transform is captured at drag start, so redo/undo are exact.
class MoveObjectCommand final : public SceneCommand
{
public:
    MoveObjectCommand(ObjectId objectId,
                      mir::Transform from,
                      mir::Transform to) noexcept
        : objectId_(objectId)
        , from_(from)
        , to_(to)
    {
    }

    void execute(mir::Scene& scene) override
    {
        apply(scene, to_);
    }

    void undo(mir::Scene& scene) override
    {
        apply(scene, from_);
    }

    [[nodiscard]] const char* description() const noexcept override
    {
        return "MOVE";
    }

private:
    void apply(mir::Scene& scene, const mir::Transform& transform)
    {
        const auto node = scene.find(objectId_);
        if (node)
            node->setTransform(transform);
    }

    ObjectId objectId_;
    mir::Transform from_;
    mir::Transform to_;
};

/// Removes one object from the scene; undo re-inserts it with its original
/// identity (the scene registry releases the id on remove and re-reserves it
/// on add).
class DeleteObjectCommand final : public SceneCommand
{
public:
    explicit DeleteObjectCommand(std::shared_ptr<mir::ModelNode> node) noexcept
        : node_(std::move(node))
    {
    }

    void execute(mir::Scene& scene) override
    {
        if (node_)
            scene.remove(node_->id());
    }

    void undo(mir::Scene& scene) override
    {
        if (node_)
            (void)scene.add(node_);
    }

    [[nodiscard]] const char* description() const noexcept override
    {
        return "DELETE";
    }

private:
    std::shared_ptr<mir::ModelNode> node_;
};

/// Stack-based history of scene commands (single undo/redo chain).
class SceneCommandHistory
{
public:
    void execute(std::unique_ptr<SceneCommand> command, mir::Scene& scene)
    {
        if (!command)
            return;

        command->execute(scene);

        // A new action invalidates the redo branch.
        redo_.clear();
        undo_.push_back(std::move(command));
    }

    bool undo(mir::Scene& scene)
    {
        if (undo_.empty())
            return false;

        auto command = std::move(undo_.back());
        undo_.pop_back();
        command->undo(scene);
        redo_.push_back(std::move(command));
        return true;
    }

    bool redo(mir::Scene& scene)
    {
        if (redo_.empty())
            return false;

        auto command = std::move(redo_.back());
        redo_.pop_back();
        command->execute(scene);
        undo_.push_back(std::move(command));
        return true;
    }

    void clear() noexcept
    {
        undo_.clear();
        redo_.clear();
    }

    [[nodiscard]] bool canUndo() const noexcept { return !undo_.empty(); }
    [[nodiscard]] bool canRedo() const noexcept { return !redo_.empty(); }

private:
    std::vector<std::unique_ptr<SceneCommand>> undo_;
    std::vector<std::unique_ptr<SceneCommand>> redo_;
};

} // namespace mir4d