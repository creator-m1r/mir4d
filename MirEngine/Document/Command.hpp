#pragma once

#include "../Core/Identity/ObjectId.hpp"
#include "../Time/Time.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mir4d
{

enum class CommandType : std::uint8_t
{
    Unknown = 0,
    CreateBox,
    Extrude,
    Move,
    Rotate,
    Scale,
    Delete,
    Rename,
    ImportMeshes,
    ExportMeshes
};

/// Compact engineering action in the 4D command stream.
struct Command
{
    std::uint64_t sequence{0};
    Time time{};
    CommandType type{CommandType::Unknown};
    ObjectId target{};
    std::vector<std::string> arguments;

    [[nodiscard]] bool isValid() const noexcept
    {
        const bool targetRequired =
            type != CommandType::CreateBox &&
            type != CommandType::Unknown &&
            type != CommandType::ImportMeshes &&
            type != CommandType::ExportMeshes;

        return type != CommandType::Unknown &&
               time.isValid() &&
               (!targetRequired || isValidObjectId(target));
    }

    [[nodiscard]] static Command make(
        std::uint64_t sequenceNumber,
        Time commandTime,
        CommandType commandType,
        ObjectId targetId = InvalidObjectId,
        std::vector<std::string> commandArguments = {})
    {
        return {
            sequenceNumber,
            commandTime,
            commandType,
            targetId,
            std::move(commandArguments)
        };
    }
};

[[nodiscard]] inline const char* commandTypeName(CommandType type) noexcept
{
    switch (type)
    {
        case CommandType::CreateBox:     return "CREATE_BOX";
        case CommandType::Extrude:       return "EXTRUDE";
        case CommandType::Move:          return "MOVE";
        case CommandType::Rotate:        return "ROTATE";
        case CommandType::Scale:         return "SCALE";
        case CommandType::Delete:        return "DELETE";
        case CommandType::Rename:        return "RENAME";
        case CommandType::ImportMeshes:  return "IMPORT_MESHES";
        case CommandType::ExportMeshes:  return "EXPORT_MESHES";
        default:                         return "UNKNOWN";
    }
}

} // namespace mir4d
