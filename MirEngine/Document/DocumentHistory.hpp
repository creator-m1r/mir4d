#pragma once

#include "Command.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mir4d
{

/// Chronological, append-only history of engineering commands.
class DocumentHistory
{
public:
    using Container = std::vector<Command>;
    using const_iterator = Container::const_iterator;

    [[nodiscard]] std::uint64_t nextSequence() const noexcept { return nextSequence_; }

    std::uint64_t append(
        Time time,
        CommandType type,
        ObjectId target = InvalidObjectId,
        std::vector<std::string> arguments = {})
    {
        const std::uint64_t sequence = nextSequence_++;
        commands_.push_back(Command::make(
            sequence,
            time,
            type,
            target,
            std::move(arguments)));
        return sequence;
    }

    bool append(const Command& command)
    {
        if (!command.isValid())
            return false;

        commands_.push_back(command);
        if (command.sequence >= nextSequence_)
            nextSequence_ = command.sequence + 1;
        return true;
    }

    void clear() noexcept
    {
        commands_.clear();
        nextSequence_ = 1;
    }

    [[nodiscard]] std::size_t size() const noexcept { return commands_.size(); }
    [[nodiscard]] bool empty() const noexcept { return commands_.empty(); }
    [[nodiscard]] const Container& commands() const noexcept { return commands_; }

    [[nodiscard]] const Command* last() const noexcept
    {
        return commands_.empty() ? nullptr : &commands_.back();
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        std::uint64_t previous = 0;
        for (const Command& command : commands_)
        {
            if (!command.isValid() || command.sequence <= previous)
                return false;
            previous = command.sequence;
        }
        return true;
    }

    [[nodiscard]] const_iterator begin() const noexcept { return commands_.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return commands_.end(); }

private:
    Container commands_;
    std::uint64_t nextSequence_{1};
};

} // namespace mir4d
