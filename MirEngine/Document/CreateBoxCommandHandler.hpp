#pragma once

#include "CommandHandler.hpp"
#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Builders/BRepMakeRectangleProfile.hpp"
#include "MirEngine/BRep/Builders/BRepExtrudeBuilder.hpp"
#include "MirEngine/BRep/Tessellator/BRepTessellator.hpp"
#include "MirEngine/BRep/Validator/BRepValidator.hpp"
#include "../Geometry/Model/Model.hpp"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string_view>

namespace mir4d
{

/// Creates a real document mesh from a rectangular profile extruded along +Z.
class CreateBoxCommandHandler final : public CommandHandler
{
public:
    [[nodiscard]] CommandResult execute(
        const Command& command,
        mir::Scene& scene) override
    {
        if (!command.isValid() || command.type != CommandType::CreateBox)
            return CommandResult::failure("Invalid CREATE_BOX command");

        if (command.arguments.size() != 3)
            return CommandResult::failure("CREATE_BOX requires width depth height");

        mir::Scalar width{}, depth{}, height{};
        if (!parse(command.arguments[0], width) ||
            !parse(command.arguments[1], depth) ||
            !parse(command.arguments[2], height))
        {
            return CommandResult::failure("CREATE_BOX dimensions are invalid");
        }

        if (!(width > 0.0) || !(depth > 0.0) || !(height > 0.0))
            return CommandResult::failure("CREATE_BOX dimensions must be positive");

        mir::BRepModel brep;
        const auto profile = mir::BRepMakeRectangleProfile::buildXY(brep, width, depth);
        if (!profile.success || !profile.wire.valid())
            return CommandResult::failure("CREATE_BOX profile construction failed");

        const auto extruded = mir::BRepExtrudeBuilder::extrudeWire(
            brep,
            profile.wire,
            mir::Vector3::unitZ(),
            height);
        if (!extruded.success || !extruded.solid.valid())
            return CommandResult::failure("CREATE_BOX extrusion failed");

        const mir::BRepValidator validator;
        if (!validator.validate(brep).ok())
            return CommandResult::failure("CREATE_BOX produced invalid B-Rep");

        const mir::TriangleMesh3 mesh =
            mir::BRepTessellator::tessellateSolid(brep, extruded.solid);
        if (!mesh.isValid())
            return CommandResult::failure("CREATE_BOX tessellation failed");

        auto model = std::make_shared<mir::Model>();
        model->setMesh(mesh);

        const auto node = scene.createNode(std::move(model));
        if (!node)
            return CommandResult::failure("CREATE_BOX failed to add object to scene");

        return CommandResult::ok(
            "Created box " + std::to_string(width) + " x " +
            std::to_string(depth) + " x " + std::to_string(height),
            node->id());
    }

private:
    [[nodiscard]] static bool parse(
        std::string_view text,
        mir::Scalar& value) noexcept
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
};

} // namespace mir4d
