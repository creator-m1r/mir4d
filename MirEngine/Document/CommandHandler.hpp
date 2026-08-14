#pragma once

#include "Command.hpp"
#include "../Geometry/Scene/Scene.hpp"

#include <string>
#include <utility>

namespace mir4d
{

/// Result of executing one document command.
struct CommandResult
{
    bool success{false};
    std::string message;
    ObjectId objectId{InvalidObjectId};

    [[nodiscard]] static CommandResult ok(
        std::string text = {},
        ObjectId id = InvalidObjectId)
    {
        return {true, std::move(text), id};
    }

    [[nodiscard]] static CommandResult failure(std::string text)
    {
        return {false, std::move(text), InvalidObjectId};
    }
};

/// Command execution boundary between the document and geometry.
/// Scene remains a legacy geometry type until the Geometry migration.
class CommandHandler
{
public:
    virtual ~CommandHandler() = default;

    [[nodiscard]] virtual CommandResult execute(
        const Command& command,
        mir::Scene& scene) = 0;
};

} // namespace mir4d
