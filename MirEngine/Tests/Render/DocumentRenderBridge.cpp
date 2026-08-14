#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Rendering/DocumentSceneRenderBridge.hpp"
#include <cassert>
#include <memory>

int main()
{
    mir4d::Document document("Render metadata bridge test");
    auto model = std::make_shared<mir::Model>();
    mir::TriangleMesh3 mesh;
    mesh.vertices = {{0.0, 0.0, 0.0}, {2.0, 0.0, 0.0}, {0.0, 3.0, 0.0}};
    mesh.triangles = {{0, 1, 2}};
    assert(mesh.isValid());
    model->setMesh(std::move(mesh));
    assert(model->isValid());
    auto node = document.scene().createNode(std::move(model));
    assert(node && node->isValid());
    const auto initialRevision = document.scene().revision();
    const auto initialContentRevision = document.scene().contentRevision();
    assert(initialRevision != 0 && initialContentRevision != 0);
    mir::rendering::DocumentSceneRenderBridge bridge;
    const auto bounds = bridge.rebuild(document.scene());
    assert(bounds.valid);
    assert(bounds.min == mir::Point3(0.0, 0.0, 0.0));
    assert(bounds.max == mir::Point3(2.0, 3.0, 0.0));
    assert(bounds.radius() > 0.0);
    assert(bridge.sourceRevision() == initialContentRevision);
    assert(bridge.objectIds().size() == 1 && bridge.objectIds().front() == node->id());
    const auto cachedBounds = bridge.rebuild(document.scene());
    assert(cachedBounds.min == bounds.min && cachedBounds.max == bounds.max);
    mir::Transform moved = node->transform();
    moved.position = {10.0, 0.0, 0.0};
    node->setTransform(moved);
    assert(document.scene().revision() == initialRevision);
    assert(document.scene().contentRevision() != initialContentRevision);
    const auto movedBounds = bridge.rebuild(document.scene());
    assert(bridge.sourceRevision() == document.scene().contentRevision());
    assert(movedBounds.min == mir::Point3(10.0, 0.0, 0.0));
    assert(movedBounds.max == mir::Point3(12.0, 3.0, 0.0));
    return 0;
}
