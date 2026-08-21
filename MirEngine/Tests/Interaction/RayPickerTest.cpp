#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Interaction/BoundingVolumeHierarchy.hpp"
#include "MirEngine/Viewport/Camera.hpp"
#include "MirEngine/Viewport/ViewportState.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <random>
#include <set>

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

namespace
{

mir::TriangleMesh3 makeBox()
{
    // Unit cube centered at the origin, spanning [-1, 1] on every axis.
    mir::TriangleMesh3 mesh;
    mesh.vertices = {
        {-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}, {1.0, 1.0, -1.0},
        {-1.0, 1.0, -1.0},  {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0},
        {1.0, 1.0, 1.0},    {-1.0, 1.0, 1.0}};
    mesh.triangles = {
        {0, 1, 2}, {0, 2, 3}, {4, 6, 5}, {4, 7, 6},
        {0, 4, 5}, {0, 5, 1}, {1, 5, 6}, {1, 6, 2},
        {2, 6, 7}, {2, 7, 3}, {3, 7, 4}, {3, 4, 0}};
    assert(mesh.isValid());
    return mesh;
}

std::shared_ptr<mir::Model> makeBoxModel()
{
    auto model = std::make_shared<mir::Model>();
    model->setMesh(makeBox());
    return model;
}

void runHierarchicalTests()
{
    // Single unit cube at the origin: deterministic geometry for sub-object
    // picking. Vertex indices follow makeBox()'s ordering.
    mir4d::Document doc("hierarchical");
    auto model = makeBoxModel();
    auto node = doc.scene().createNode(model);
    const auto id = node->id();

    const mir::Point3 v0{-1.0, -1.0, -1.0};                  // vertex index 0
    const mir::Point3 edgeMid{0.0, -1.0, -1.0};             // midpoint of edge (v0,v1): tri 0, edge 0
    const mir::Point3 faceMid{1.0 / 3.0, -1.0 / 3.0, -1.0}; // centroid of triangle 0

    auto rayThrough = [](const mir::Point3& p) {
        return mir::PickRay{mir::Point3{p.x, p.y, p.z + 0.05}, mir::Vector3{0.0, 0.0, -1.0}};
    };

    // Vertex mode (filter index 4) through vertex 0 -> Vertex, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(v0), mir::makePickFilter(4));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Vertex);
        assert(r.objectId == id);
        assert(r.elementId == 0);
    }

    // Edge mode (filter index 3) through edge midpoint -> Edge, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(edgeMid), mir::makePickFilter(3));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Edge);
        assert(r.elementId == 0);
    }

    // Face mode (filter index 2) through face centroid -> Face.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(2));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Face);
        assert(r.objectId == id);
    }

    // Body mode (filter index 1) through centroid -> Body, elementId 0.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(1));
        assert(r.hit());
        assert(r.kind == mir::PickKind::Body);
        assert(r.elementId == 0);
    }

    // Vertex mode (filter 4) but cursor far from any vertex -> strict miss.
    {
        const auto r = mir::RayPicker::pick(doc.scene(), rayThrough(faceMid), mir::makePickFilter(4));
        assert(!r.hit());
        assert(r.kind == mir::PickKind::None);
    }
}

} // namespace

namespace
{

void runBoxSelectionTests()
{
    mir4d::Document doc("box-selection");

    auto leftNode = doc.scene().createNode(makeBoxModel());
    mir::Transform leftT;
    leftT.position = {-18.0, 0.0, 0.0};
    leftNode->setTransform(leftT);

    auto rightNode = doc.scene().createNode(makeBoxModel());
    mir::Transform rightT;
    rightT.position = {18.0, 0.0, 0.0};
    rightNode->setTransform(rightT);

    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 20.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;

    mir::ViewportState state;
    state.camera = camera;
    state.resize(width, height);

    mir::Scene& scene = doc.scene();

    // Right-half rectangle intersects only the right node.
    state.selectInRect(scene, width * 0.55f, 0.0f, width * 0.95f,
                       static_cast<float>(height), false);
    assert(state.multiSelectionCount() == 1);
    assert(state.multiSelectionAt(0) == rightNode->id());
    assert(state.selection.primary() == rightNode->id());

    // Whole-viewport rectangle selects both cubes.
    state.selectInRect(scene, 0.0f, 0.0f, static_cast<float>(width),
                       static_cast<float>(height), false);
    assert(state.multiSelectionCount() == 2);

    // Additive selection over the right half unions with the existing set.
    state.selectInRect(scene, width * 0.55f, 0.0f, width * 0.95f,
                       static_cast<float>(height), true);
    assert(state.multiSelectionCount() == 2);

    // A rectangle in empty space clears the selection (non-additive).
    state.selectInRect(scene, 1.0f, 1.0f, 2.0f, 2.0f, false);
    assert(state.multiSelectionCount() == 0);
    assert(state.selection.primary() == mir4d::InvalidObjectId);
}

} // namespace

int main()
{
    mir4d::Document document("RayPicker Y-orientation test");

    // Upper cube sits at world +Y (renders at the top of the viewport).
    // It is placed near the top edge of the orthographic frustum so the
    // top-edge picking ray passes through its center.
    auto topModel = makeBoxModel();
    auto topNode = document.scene().createNode(topModel);
    mir::Transform topTransform;
    topTransform.position = {0.0, 18.0, 0.0};
    topNode->setTransform(topTransform);

    // Lower cube sits at world -Y (renders at the bottom of the viewport).
    auto bottomModel = makeBoxModel();
    auto bottomNode = document.scene().createNode(bottomModel);
    mir::Transform bottomTransform;
    bottomTransform.position = {0.0, -18.0, 0.0};
    bottomNode->setTransform(bottomTransform);

    // Orthographic camera looking straight down -Z with up = +Y, so world +Y
    // maps linearly to screen-up. With the Z-up convention phi is measured from
    // +Z, so a top-down view (eye on +Z, looking -Z) is phi = 0. theta = 0.
    // distance = 20 => ortho half-height = 20, so world Y at z=0 equals
    // ndcY * 20. A ray at ndcY = 0.9 (screenY = 0.95*H) hits world Y = 18.
    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 20.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;
    const float cx = width * 0.5f;

    // Top of the viewport (screenY measured from the bottom in the engine's
    // view-local convention) must resolve to the upper cube.
    const auto topHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.95f, width, height);
    assert(topHit.hit());
    assert(topHit.objectId == topNode->id());

    // Bottom of the viewport must resolve to the lower cube.
    const auto bottomHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.05f, width, height);
    assert(bottomHit.hit());
    assert(bottomHit.objectId == bottomNode->id());

    // The two hits must be different objects.
    assert(topHit.objectId != bottomHit.objectId);

    runHierarchicalTests();
    runBoxSelectionTests();

    std::cout << "MIR4D RAYPICKER: OK\n";
    return 0;
}
