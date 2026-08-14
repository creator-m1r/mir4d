#pragma once

#include "../Core/Identity/ObjectRegistry.hpp"
#include "ObjectStore.hpp"
#include "CommandHandler.hpp"
#include "DocumentHistory.hpp"
#include "DocumentSnapshot.hpp"
#include "../Time/Time.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace mir4d
{

/// Root in-memory representation of a MIR 4D engineering project.
///
/// Document owns project identity, object storage, command history,
/// deterministic time and revision state. ObjectStore is the canonical
/// ownership boundary; its current Scene implementation is transitional.
class Document
{
public:
    Document()
        : store_(registry_)
    {
    }

    explicit Document(std::string name)
        : name_(std::move(name)), store_(registry_)
    {
    }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    void setName(std::string name)
    {
        name_ = std::move(name);
        markModified();
    }

    [[nodiscard]] ObjectStore& objects() noexcept { return store_; }
    [[nodiscard]] const ObjectStore& objects() const noexcept { return store_; }

    /// Transitional compatibility accessor. New code should use objects().
    [[nodiscard]] mir::Scene& scene() noexcept { return store_.scene(); }
    [[nodiscard]] const mir::Scene& scene() const noexcept { return store_.scene(); }

    [[nodiscard]] DocumentHistory& history() noexcept { return history_; }
    [[nodiscard]] const DocumentHistory& history() const noexcept { return history_; }

    [[nodiscard]] Time time() const noexcept { return time_; }

    void setTime(Time time) noexcept
    {
        time_ = time;
        markModified();
    }

    void advanceTime(double seconds) noexcept
    {
        time_.advance(seconds);
        markModified();
    }

    [[nodiscard]] ObjectRegistry& objectRegistry() noexcept { return registry_; }
    [[nodiscard]] const ObjectRegistry& objectRegistry() const noexcept { return registry_; }

    [[nodiscard]] ObjectId allocateObjectId()
    {
        const ObjectId id = registry_.allocate();
        markModified();
        return id;
    }

    bool reserveObjectId(ObjectId id)
    {
        const bool reserved = registry_.reserve(id);
        if (reserved)
            markModified();
        return reserved;
    }

    bool releaseObjectId(ObjectId id) noexcept
    {
        const bool released = registry_.release(id);
        if (released)
            markModified();
        return released;
    }

    [[nodiscard]] DocumentSnapshot snapshot() const
    {
        return DocumentSnapshot::capture(store_.scene());
    }

    [[nodiscard]] bool restoreSnapshot(const DocumentSnapshot& snapshot) noexcept
    {
        if (!snapshot.restore(store_.scene()))
            return false;

        advanceRevision();
        return true;
    }

    [[nodiscard]] CommandResult execute(
        const Command& command,
        CommandHandler& handler)
    {
        if (!command.isValid())
            return CommandResult::failure("Invalid command");

        if (command.time.seconds() < time_.seconds())
            return CommandResult::failure("Command time cannot be earlier than document time");

        CommandResult result = handler.execute(command, store_.scene());
        if (!result.success)
            return result;

        const ObjectId target =
            result.objectId != InvalidObjectId ? result.objectId : command.target;

        const Command historyCommand = Command::make(
            history_.nextSequence(),
            command.time,
            command.type,
            target,
            command.arguments);

        if (!history_.append(historyCommand))
            return CommandResult::failure("Command executed but could not be recorded in history");

        time_ = command.time;
        advanceRevision();
        return result;
    }

    void markModified() noexcept { modified_ = true; }
    void clearModified() noexcept { modified_ = false; }
    [[nodiscard]] bool isModified() const noexcept { return modified_; }

    [[nodiscard]] std::uint64_t revision() const noexcept { return revision_; }

    void advanceRevision() noexcept
    {
        ++revision_;
        modified_ = true;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return store_.isValid() && time_.isValid() && history_.isValid();
    }

private:
    std::string name_;
    ObjectRegistry registry_;
    ObjectStore store_;
    DocumentHistory history_;
    Time time_{};
    std::uint64_t revision_{0};
    bool modified_{false};
};

} // namespace mir4d
