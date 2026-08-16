#include "MirEngine/Document/Document.hpp"
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Viewport/Camera.hpp"
#include "MirEngine/Math/Transform.hpp"

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

    std::cout << "MIR4D RAYPICKER: OK\n";
    return 0;
}
