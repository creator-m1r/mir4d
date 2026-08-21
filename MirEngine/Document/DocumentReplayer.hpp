#pragma once

#include "CommandExecutor.hpp"
#include "Document.hpp"

#include <cstddef>

namespace mir
{

class DocumentReplayer
{
public:
    explicit DocumentReplayer(const CommandExecutor& executor) noexcept
        : executor_(executor)
    {
    }

    [[nodiscard]] std::size_t commandCountAt(Time time) const noexcept
    {
        return executor_.countAtOrBefore(time);
    }

    [[nodiscard]] bool canReplayTo(Time time) const noexcept
    {
        return time.isValid();
    }

private:
    const CommandExecutor& executor_;
};

}
