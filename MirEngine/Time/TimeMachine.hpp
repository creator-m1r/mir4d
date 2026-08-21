#pragma once

#include "../Document/Document.hpp"
#include "../Document/DocumentSnapshot.hpp"
#include "../Document/CommandHandler.hpp"

#include <cstddef>
#include <vector>

namespace mir4d
{

class TimeMachine
{
public:
    explicit TimeMachine(Document& document) noexcept
        : document_(document)
        , handler_(nullptr)
    {
    }

    TimeMachine(Document& document, CommandHandler& handler) noexcept
        : document_(document)
        , handler_(&handler)
    {
    }

    [[nodiscard]] Time currentTime() const noexcept
    {
        return document_.time();
    }

    [[nodiscard]] bool captureSnapshot() noexcept
    {
        checkpoints_.push_back({document_.time(), document_.snapshot()});
        return true;
    }

    [[nodiscard]] bool seek(Time target) noexcept
    {
        if (!target.isValid())
            return false;

        const Checkpoint* checkpoint = nearestCheckpoint(target);
        if (!checkpoint)
            return false;

        if (!document_.restoreSnapshot(checkpoint->snapshot))
            return false;

        if (!handler_)
        {
            if (checkpoint->time.seconds() != target.seconds())
                return false;

            document_.setTime(target);
            return true;
        }

        for (const Command& command : document_.history())
        {
            const double commandTime = command.time.seconds();

            if (commandTime <= checkpoint->time.seconds())
                continue;
            if (commandTime > target.seconds())
                break;

            const CommandResult result = handler_->execute(command, document_.scene());
            if (!result.success)
                return false;
        }

        document_.setTime(target);
        return true;
    }

    [[nodiscard]] bool seekSeconds(double seconds) noexcept
    {
        return seek(Time(seconds));
    }

    [[nodiscard]] std::size_t commandCountAt(Time target) const noexcept
    {
        if (!target.isValid())
            return 0;

        std::size_t count = 0;
        for (const Command& command : document_.history())
        {
            if (command.time.seconds() <= target.seconds())
                ++count;
            else
                break;
        }
        return count;
    }

    [[nodiscard]] std::size_t commandCountAtSeconds(double seconds) const noexcept
    {
        return commandCountAt(Time(seconds));
    }

    [[nodiscard]] std::size_t checkpointCount() const noexcept
    {
        return checkpoints_.size();
    }

private:
    struct Checkpoint
    {
        Time time;
        DocumentSnapshot snapshot;
    };

    [[nodiscard]] const Checkpoint* nearestCheckpoint(Time target) const noexcept
    {
        const Checkpoint* result = nullptr;
        double bestTime = -1.0;

        for (const auto& checkpoint : checkpoints_)
        {
            const double value = checkpoint.time.seconds();
            if (value <= target.seconds() && value >= bestTime)
            {
                bestTime = value;
                result = &checkpoint;
            }
        }

        return result;
    }

    Document& document_;
    CommandHandler* handler_;
    std::vector<Checkpoint> checkpoints_;
};

}
