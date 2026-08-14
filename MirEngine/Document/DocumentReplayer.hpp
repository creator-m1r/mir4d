#pragma once

#include "CommandExecutor.hpp"
#include "Document.hpp"

#include <cstddef>

namespace mir
{

/// Rebuilds the logical document timeline by replaying commands in order.
///
/// Geometry-specific command handlers are intentionally supplied by the next
/// layer. This class currently provides deterministic timeline traversal.
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

} // namespace mir
