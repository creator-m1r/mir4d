#pragma once

#include "Command.hpp"
#include "../Time/Time.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace mir
{

class CommandExecutor
{
public:
    using CommandList = std::vector<Command>;

    [[nodiscard]] const CommandList& commands() const noexcept
    {
        return commands_;
    }

    void clear() noexcept
    {
        commands_.clear();
        cursor_ = 0;
    }

    void append(const Command& command)
    {
        commands_.push_back(command);
        cursor_ = commands_.size();
    }

    [[nodiscard]] std::size_t cursor() const noexcept
    {
        return cursor_;
    }

    [[nodiscard]] std::size_t countAtOrBefore(Time time) const noexcept
    {
        std::size_t count = 0;
        for (const auto& command : commands_)
        {
            if (command.time().seconds() <= time.seconds())
                ++count;
            else
                break;
        }
        return count;
    }

    [[nodiscard]] bool seek(Time time) noexcept
    {
        if (!time.isValid())
            return false;
        cursor_ = countAtOrBefore(time);
        return true;
    }

    [[nodiscard]] std::vector<Command> replay(Time time) const
    {
        const std::size_t count = countAtOrBefore(time);
        return CommandList(commands_.begin(), commands_.begin() + count);
    }

private:
    CommandList commands_;
    std::size_t cursor_{0};
};

}
