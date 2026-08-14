#include "MeshImportCommandHandler.hpp"

#include "../../Geometry/Model/Model.hpp"

#include <memory>
#include <stdexcept>

namespace mir::io
{
namespace
{
[[nodiscard]] ImportOptions optionsFromArguments(const std::vector<std::string>& arguments)
{
    ImportOptions options;
    for (const std::string& argument : arguments)
    {
        if (argument.rfind("generateNormals=", 0) == 0)
            options.generateNormals = argument.back() == '1';
        else if (argument.rfind("triangulate=", 0) == 0)
            options.triangulate = argument.back() == '1';
        else if (argument.rfind("joinVertices=", 0) == 0)
            options.joinIdenticalVertices = argument.back() == '1';
        else if (argument.rfind("unitScale=", 0) == 0)
        {
            try
            {
                options.unitScale = std::stod(argument.substr(10));
            }
            catch (...)
            {
                options.unitScale = 1.0;
            }
        }
    }
    return options;
}
} // namespace

mir4d::CommandResult MeshImportCommandHandler::execute(
    const mir4d::Command& command,
    mir::Scene& scene)
{
    if (command.type != mir4d::CommandType::ImportMeshes)
        return mir4d::CommandResult::failure("MeshImportCommandHandler received unsupported command");

    if (command.arguments.empty() || command.arguments.front().empty())
        return mir4d::CommandResult::failure("Import command has no source path");

    const std::string& path = command.arguments.front();
    const ImportResult result = service_.importFile(
        path,
        optionsFromArguments(command.arguments));

    if (!result.ok())
        return mir4d::CommandResult::failure(result.error.empty()
            ? "Mesh import failed"
            : result.error);

    auto model = std::make_shared<mir::Model>();
    model->setMesh(*result.mesh);

    auto node = scene.createNode(std::move(model));
    if (!node)
        return mir4d::CommandResult::failure("Imported mesh could not be added to the scene");

    return mir4d::CommandResult::ok(
        "Imported " + std::to_string(result.triangleCount) +
        " triangles from " + path,
        node->id());
}

} // namespace mir::io
