#include "MirEngine/IO/ImportService.hpp"
#include "MirEngine/IO/ExportService.hpp"
#include "MirEngine/IO/Commands/ImportMeshesCommand.hpp"
#include "MirEngine/IO/Commands/MeshImportCommandHandler.hpp"
#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Rendering/DocumentSceneRenderBridge.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Scene/../Model/ModelNode.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    using namespace mir4d;
    using namespace mir::io;

    const auto fixturePath = std::filesystem::path(__FILE__).parent_path() / "Fixtures" / "triangle.stl";
    const auto directory = std::filesystem::temp_directory_path() / "mir4d_stl_roundtrip";
    std::filesystem::create_directories(directory);
    const auto binaryPath = directory / "triangle_binary.stl";
    const auto selectedPath = directory / "triangle_selected.stl";

    ImportService importService;
    const ImportResult imported = importService.importFile(fixturePath.string());
    assert(imported.ok() && imported.format == Format::StlAscii && imported.triangleCount == 1 && imported.mesh && imported.mesh->isValid());

    Document document("STL Round Trip");
    MeshImportCommandHandler handler;
    const Command command = ImportMeshesCommand::make(document.time(), fixturePath.string());
    const CommandResult commandResult = document.execute(command, handler);
    assert(commandResult.success && isValidObjectId(commandResult.objectId));
    assert(document.scene().size() == 1 && document.scene().nodes().front()->model()->hasMesh());

    mir::rendering::DocumentSceneRenderBridge bridge;
    const auto bounds = bridge.rebuild(document.scene());
    assert(bounds.valid && bounds.radius() > 0.0);
    assert(bridge.hasCachedScene());
    assert(bridge.sourceRevision() == document.scene().contentRevision());
    assert(bridge.objectIds().size() == 1 && bridge.objectIds().front() == commandResult.objectId);

    ExportService exportService;
    ExportOptions allOptions;
    allOptions.binaryStl = true;
    const ExportResult exported = exportService.exportFile(binaryPath.string(), document, allOptions);
    assert(exported.ok() && exported.format == Format::StlBinary && exported.triangleCount == imported.triangleCount);

    const ImportResult importedBinary = importService.importFile(binaryPath.string());
    assert(importedBinary.ok() && importedBinary.triangleCount == imported.triangleCount && importedBinary.mesh->isValid());

    ExportOptions selectionOptions;
    selectionOptions.binaryStl = true;
    selectionOptions.selectionOnly = true;
    selectionOptions.selection = {commandResult.objectId};
    const ExportResult selectedExport = exportService.exportFile(selectedPath.string(), document, selectionOptions);
    assert(selectedExport.ok() && selectedExport.triangleCount == imported.triangleCount);

    const ImportResult importedSelected = importService.importFile(selectedPath.string());
    assert(importedSelected.ok() && importedSelected.triangleCount == imported.triangleCount && importedSelected.mesh->isValid());

    std::filesystem::remove_all(directory);
    std::cout << "MIR4D STL round-trip: OK\n";
    return 0;
}
