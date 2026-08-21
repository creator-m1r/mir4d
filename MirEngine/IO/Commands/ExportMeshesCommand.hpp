#pragma once

#include "../../Document/Command.hpp"
#include "../ExportOptions.hpp"

#include <string>
#include <utility>
#include <vector>

namespace mir::io
{

/// Factory for the persistent mesh export action.
struct ExportMeshesCommand
{
    [[nodiscard]] static Command make(
        Time time,
        std::string path,
        const ExportOptions& options = {})
    {
        std::vector<std::string> arguments;
        arguments.reserve(3 + options.selection.size());
        arguments.push_back(std::move(path));
        arguments.push_back(options.binaryStl ? "binaryStl=1" : "binaryStl=0");
        arguments.push_back(options.selectionOnly ? "selectionOnly=1" : "selectionOnly=0");
        arguments.push_back("unitScale=" + std::to_string(options.unitScale));
        for (const ObjectId id : options.selection)
            arguments.push_back("selection=" + std::to_string(id));

        return Command::make(
            0,
            time,
            CommandType::ExportMeshes,
            InvalidObjectId,
            std::move(arguments));
    }
};

} // namespace mir::io
