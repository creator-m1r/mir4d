#pragma once

#include "Command.hpp"
#include "../Geometry/Scene/Scene.hpp"

#include <string>
#include <utility>

namespace mir4d
{

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

class CommandHandler
{
public:
    virtual ~CommandHandler() = default;

    [[nodiscard]] virtual CommandResult execute(
        const Command& command,
        mir::Scene& scene) = 0;
};

}
