#pragma once

// MirEngine/BRep/Commands/BRepCommandHandler.hpp

#include "MirEngine/Document/CommandHandler.hpp"
#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Commands/BRepSceneBridge.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

namespace mir4d
{

class BRepCommandHandler final : public CommandHandler
{
public:
    explicit BRepCommandHandler(mir::BRepModel& brep) noexcept
        : brep_(brep)
    {
    }

    [[nodiscard]] CommandResult execute(
        const Command& command,
        mir::Scene& scene) override
    {
        if (!command.isValid())
            return CommandResult::failure("Invalid command");

        switch (command.type)
        {
            case CommandType::CreateBox: return createBox(command, scene);
            case CommandType::Move: return moveObject(command, scene);
            case CommandType::Delete: return deleteObject(command, scene);
            case CommandType::Extrude:
                return CommandResult::failure("EXTRUDE via command payload is reserved for feature layer");
            default:
                return CommandResult::failure(
                    std::string("Unsupported command: ") + commandTypeName(command.type));
        }
    }

private:
    [[nodiscard]] static bool parseScalar(std::string_view text, mir::Scalar& value) noexcept
    {
        if (text.empty())
            return false;

        const char* last = text.data() + text.size();
        char* end = nullptr;
        errno = 0;
        const double parsed = std::strtod(text.data(), &end);
        if (end != last || errno == ERANGE || !std::isfinite(parsed))
            return false;

        value = static_cast<mir::Scalar>(parsed);
        return true;
    }

    [[nodiscard]] CommandResult createBox(const Command& command, mir::Scene& scene)
    {
        if (command.arguments.size() != 3)
            return CommandResult::failure("CREATE_BOX requires width depth height");

        mir::Scalar sizeX{}, sizeY{}, sizeZ{};
        if (!parseScalar(command.arguments[0], sizeX) ||
            !parseScalar(command.arguments[1], sizeY) ||
            !parseScalar(command.arguments[2], sizeZ))
            return CommandResult::failure("CREATE_BOX has invalid dimensions");

        const BRepSceneInsertResult inserted =
            BRepSceneBridge::createBox(scene, brep_, sizeX, sizeY, sizeZ);

        if (!inserted.success)
            return CommandResult::failure("CREATE_BOX BRep pipeline failed");

        if (auto node = scene.find(inserted.objectId))
            node->setBrep(std::make_shared<mir::BRepModel>(brep_));

        return CommandResult::ok(
            "CREATE_BOX created object " + std::to_string(inserted.objectId),
            inserted.objectId);
    }

    [[nodiscard]] static CommandResult moveObject(const Command& command, mir::Scene& scene)
    {
        if (command.arguments.size() != 3)
            return CommandResult::failure("MOVE requires three coordinates");

        auto node = scene.find(command.target);
        if (!node)
            return CommandResult::failure("MOVE target not found");

        mir::Scalar x{}, y{}, z{};
        if (!parseScalar(command.arguments[0], x) ||
            !parseScalar(command.arguments[1], y) ||
            !parseScalar(command.arguments[2], z))
            return CommandResult::failure("MOVE has invalid coordinates");

        mir::Transform transform = node->transform();
        transform.position = {x, y, z};
        node->setTransform(transform);
        return CommandResult::ok("MOVE applied", command.target);
    }

    [[nodiscard]] static CommandResult deleteObject(const Command& command, mir::Scene& scene)
    {
        if (!scene.remove(command.target))
            return CommandResult::failure("DELETE target not found");
        return CommandResult::ok("Object deleted", command.target);
    }

    mir::BRepModel& brep_;
};

} // namespace mir4d
