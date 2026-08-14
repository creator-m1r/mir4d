#pragma once

#include "CommandHandler.hpp"
#include "../Geometry/Primitives/Box.hpp"
#include "../Geometry/Tessellation/TriangleMesh.hpp"

#include <charconv>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace mir
{

/// Built-in deterministic handler for scene editing and primitive creation.
class BasicCommandHandler final : public CommandHandler
{
public:
    [[nodiscard]] CommandResult execute(
        const Command& command,
        Scene3& scene) override
    {
        if (!command.isValid())
            return CommandResult::failure("Invalid command");

        switch (command.type)
        {
            case CommandType::CreateBox:
                return createBox(command, scene);
            case CommandType::Move:
                return applyVector(command, scene, "MOVE", VectorOperation::Move);
            case CommandType::Rotate:
                return applyVector(command, scene, "ROTATE", VectorOperation::Rotate);
            case CommandType::Scale:
                return applyVector(command, scene, "SCALE", VectorOperation::Scale);
            case CommandType::Delete:
                return deleteObject(command, scene);
            case CommandType::Extrude:
                return CommandResult::failure("EXTRUDE requires the solid operation layer");
            case CommandType::Rename:
                return CommandResult::failure("RENAME requires object naming support");
            default:
                return CommandResult::failure(
                    "Unsupported command: " + std::string(commandTypeName(command.type)));
        }
    }

private:
    enum class VectorOperation { Move, Rotate, Scale };

    [[nodiscard]] static bool parseScalar(
        std::string_view text,
        Scalar& value) noexcept
    {
        const char* first = text.data();
        const char* last = first + text.size();
        const auto result = std::from_chars(first, last, value);
        return result.ec == std::errc{} && result.ptr == last && std::isfinite(value);
    }

    [[nodiscard]] static bool parseVector(
        const Command& command,
        Vector3& value) noexcept
    {
        if (command.arguments.size() != 3)
            return false;

        return parseScalar(command.arguments[0], value.x) &&
               parseScalar(command.arguments[1], value.y) &&
               parseScalar(command.arguments[2], value.z) &&
               value.isFinite();
    }

    [[nodiscard]] static CommandResult createBox(
        const Command& command,
        Scene3& scene)
    {
        if (command.arguments.size() != 3)
            return CommandResult::failure("CREATE_BOX requires width depth height");

        Scalar width{}, depth{}, height{};
        if (!parseScalar(command.arguments[0], width) ||
            !parseScalar(command.arguments[1], depth) ||
            !parseScalar(command.arguments[2], height))
        {
            return CommandResult::failure("CREATE_BOX contains an invalid dimension");
        }

        const Box box(width, depth, height);
        if (!box.isValid())
            return CommandResult::failure(
                "CREATE_BOX dimensions must be finite and positive");

        const auto vertices = box.vertices();
        const Solid3 solid = box.build();
        if (!solid.isValid())
            return CommandResult::failure("CREATE_BOX failed to build solid");

        auto model = std::make_shared<Model3>();
        model->setSolid(solid);

        TriangleMesh3 mesh;
        mesh.vertices.assign(vertices.begin(), vertices.end());
        mesh.triangles = {
            {0, 2, 1}, {0, 3, 2},
            {4, 5, 6}, {4, 6, 7},
            {0, 1, 5}, {0, 5, 4},
            {1, 2, 6}, {1, 6, 5},
            {2, 3, 7}, {2, 7, 6},
            {3, 0, 4}, {3, 4, 7}
        };

        if (!mesh.isValid())
            return CommandResult::failure("CREATE_BOX produced an invalid mesh");

        model->setMesh(std::move(mesh));

        const auto node = scene.createNode(std::move(model));
        if (!node)
            return CommandResult::failure("CREATE_BOX failed to add object to scene");

        const ObjectId id = node->id();
        return CommandResult::ok(
            "CREATE_BOX created object " + std::to_string(id), id);
    }

    [[nodiscard]] static CommandResult applyVector(
        const Command& command,
        Scene3& scene,
        const char* name,
        VectorOperation operation)
    {
        const auto node = scene.find(command.target);
        if (!node)
            return CommandResult::failure(std::string(name) + " target object was not found");

        Vector3 value;
        if (!parseVector(command, value))
            return CommandResult::failure(std::string(name) + " requires 3 finite numeric arguments");

        Transform3 transform = node->transform();

        switch (operation)
        {
            case VectorOperation::Move:
                transform.position += value;
                break;
            case VectorOperation::Rotate:
                transform.rotationRadians += value;
                break;
            case VectorOperation::Scale:
                if (value.x == Scalar(0.0) || value.y == Scalar(0.0) || value.z == Scalar(0.0))
                    return CommandResult::failure("SCALE components cannot be zero");
                transform.scale = Vector3::componentMul(transform.scale, value);
                break;
        }

        if (!transform.isValid())
            return CommandResult::failure(std::string(name) + " produced an invalid transform");

        node->setTransform(transform);
        return CommandResult::ok(std::string(name) + " applied", command.target);
    }

    [[nodiscard]] static CommandResult deleteObject(
        const Command& command,
        Scene3& scene)
    {
        if (!scene.remove(command.target))
            return CommandResult::failure("DELETE target object was not found");

        return CommandResult::ok("Object deleted", command.target);
    }
};

} // namespace mir
