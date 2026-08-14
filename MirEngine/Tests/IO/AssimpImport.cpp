#include "MirEngine/IO/ImportService.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

int main()
{
    using namespace mir::io;

    const auto fixturePath =
        std::filesystem::path(__FILE__).parent_path() / "Fixtures" / "cube.obj";

    ImportService importService;
    const ImportResult imported = importService.importFile(fixturePath.string());

    assert(imported.ok());
    assert(imported.format == Format::Obj);
    assert(imported.mesh && imported.mesh->isValid());
    assert(imported.triangleCount > 0);

    std::cout << "MIR4D Assimp import (OBJ): OK, triangles="
              << imported.triangleCount << "\n";
    return 0;
}
