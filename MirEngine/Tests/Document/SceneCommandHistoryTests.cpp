#include "MirEngine/Document/SceneCommandHistory.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"

#include <cassert>
#include <memory>

namespace
{

std::shared_ptr<mir::Model> makeModel()
{
    auto model = std::make_shared<mir::Model>();
    mir::TriangleMesh3 mesh;
    mesh.vertices = {
        {0, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0}};
    mesh.triangles = {
        {0, 1, 2, 0},
        {0, 2, 3, 0}};
    model->setMesh(mesh);
    return model;
}

}

int main()
{
    mir::Scene scene;
    mir4d::SceneCommandHistory history;

    const auto nodeA = scene.createNode(makeModel());
    const auto nodeB = scene.createNode(makeModel());
    assert(scene.size() == 2);
    assert(mir4d::isValidObjectId(nodeA->id()));
    assert(mir4d::isValidObjectId(nodeB->id()));

    const mir4d::Transform from = nodeA->transform();
    mir4d::Transform to = from;
    to.position = {5, 2, 1};

    history.execute(
        std::make_unique<mir4d::MoveObjectCommand>(nodeA->id(), from, to),
        scene);
    assert(nodeA->transform() == to);
    assert(history.canUndo() && !history.canRedo());

    assert(history.undo(scene));
    assert(nodeA->transform() == from);
    assert(history.canRedo());

    assert(history.redo(scene));
    assert(nodeA->transform() == to);
    assert(history.canUndo() && !history.canRedo());

    const mir4d::ObjectId idB = nodeB->id();
    history.execute(
        std::make_unique<mir4d::DeleteObjectCommand>(nodeB),
        scene);
    assert(scene.size() == 1);
    assert(!scene.contains(idB));
    assert(history.canUndo());

    assert(history.undo(scene));
    assert(scene.size() == 2);
    assert(scene.contains(idB));
    assert(scene.find(idB) != nullptr);

    assert(history.redo(scene));
    assert(scene.size() == 1);
    assert(!scene.contains(idB));

    history.execute(
        std::make_unique<mir4d::DeleteObjectCommand>(scene.find(nodeA->id())),
        scene);
    assert(scene.size() == 0);
    assert(!history.canRedo());

    history.clear();
    assert(!history.canUndo());
    assert(!history.canRedo());

    const auto node = scene.createNode(makeModel());
    const auto original = node->model()->mesh().vertices;
    auto modified = original;
    for (auto& v : modified)
        v = v + mir::Vector3(0, 0, 2);

    auto equalVec = [](const std::vector<mir::Point3>& a,
                       const std::vector<mir::Point3>& b) -> bool {
        if (a.size() != b.size())
            return false;
        for (std::size_t i = 0; i < a.size(); ++i)
            if (a[i].x != b[i].x || a[i].y != b[i].y || a[i].z != b[i].z)
                return false;
        return true;
    };

    history.execute(
        std::make_unique<mir4d::DeformObjectCommand>(node->id(), original, modified),
        scene);
    assert(equalVec(node->model()->mesh().vertices, modified));
    assert(history.canUndo() && !history.canRedo());

    assert(history.undo(scene));
    assert(equalVec(node->model()->mesh().vertices, original));
    assert(history.canRedo());

    assert(history.redo(scene));
    assert(equalVec(node->model()->mesh().vertices, modified));

    return 0;
}
