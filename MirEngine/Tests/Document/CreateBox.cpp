#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Document/CreateBoxCommandHandler.hpp"
#include <cassert>
#include <iostream>

int main()
{
    mir4d::Document document("Create Box Test");
    mir4d::CreateBoxCommandHandler handler;
    const mir4d::Command command = mir4d::Command::make(
        0,
        document.time(),
        mir4d::CommandType::CreateBox,
        mir4d::InvalidObjectId,
        {"100", "60", "40"});

    const mir4d::CommandResult result = document.execute(command, handler);
    assert(result.success);
    assert(mir4d::isValidObjectId(result.objectId));
    assert(document.scene().size() == 1);
    assert(document.history().size() == 1);

    const auto node = document.scene().find(result.objectId);
    assert(node && node->model() && node->model()->hasMesh());

    const auto& mesh = node->model()->mesh();
    assert(mesh.isValid());
    assert(mesh.vertices.size() >= 8);
    assert(mesh.triangles.size() >= 12);

    std::cout << "MIR4D CREATE_BOX: OK\n";
    return 0;
}
