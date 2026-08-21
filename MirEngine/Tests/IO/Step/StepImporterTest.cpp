#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Geometry/Model/ModelNode.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/IO/Step/StepExporter.hpp"
#include "MirEngine/IO/Step/StepImporter.hpp"
#include "MirEngine/IO/ExportOptions.hpp"
#include "MirEngine/IO/ImportOptions.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main()
{
    const fs::path path = fs::temp_directory_path() / "mir4d_step_roundtrip.step";

    mir::TriangleMesh3 mesh;
    mesh.vertices = {
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}};
    mesh.triangles = {
        {0, 1, 2},
        {0, 1, 3},
        {0, 2, 3},
        {1, 2, 3}};
    mesh.normals.resize(mesh.vertices.size(), mir::Vector3{0.0, 0.0, 1.0});
    assert(mesh.isValid());

    mir4d::Document document{"STEP RoundTrip"};
    auto model = std::make_shared<mir::Model>();
    model->setMesh(mesh);
    (void)document.scene().add(std::make_shared<mir::ModelNode>(model));

    mir::io::ExportOptions exportOptions;
    exportOptions.selectionOnly = false;
    const auto exportResult =
        mir::io::step::StepExporter{}.exportTo(path.string(), document, exportOptions);

    std::cout << "STEP export: triangles=" << exportResult.triangleCount
              << " error='" << exportResult.error << "'\n";
    assert(exportResult.error.empty());
    assert(exportResult.triangleCount == 4);
    assert(fs::exists(path));

    const auto importResult =
        mir::io::step::StepImporter{}.importFile(path.string());

    std::cout << "STEP import: triangles=" << importResult.triangleCount
              << " error='" << importResult.error << "'\n";
    assert(importResult.ok());
    assert(importResult.triangleCount == 4);
    assert(importResult.mesh != nullptr);
    assert(importResult.mesh->triangles.size() == 4);

    fs::remove(path);
    std::cout << "STEP round-trip OK\n";
    return 0;
}
