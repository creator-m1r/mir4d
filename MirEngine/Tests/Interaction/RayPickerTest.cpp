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

}

int main()
{
    mir4d::Document document("RayPicker Y-orientation test");

    auto topModel = makeBoxModel();
    auto topNode = document.scene().createNode(topModel);
    mir::Transform topTransform;
    topTransform.position = {0.0, 18.0, 0.0};
    topNode->setTransform(topTransform);

    auto bottomModel = makeBoxModel();
    auto bottomNode = document.scene().createNode(bottomModel);
    mir::Transform bottomTransform;
    bottomTransform.position = {0.0, -18.0, 0.0};
    bottomNode->setTransform(bottomTransform);

    mir::Camera camera;
    camera.setTarget({0.0, 0.0, 0.0});
    camera.setOrbit(0.0, 0.0, 20.0);
    camera.setProjection(mir::CameraProjection::Orthographic);
    camera.setAspect(800.0 / 600.0);

    const std::uint32_t width = 800;
    const std::uint32_t height = 600;
    const float cx = width * 0.5f;

    const auto topHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.95f, width, height);
    assert(topHit.hit());
    assert(topHit.objectId == topNode->id());

    const auto bottomHit =
        mir::RayPicker::pick(document.scene(), camera, cx, height * 0.05f, width, height);
    assert(bottomHit.hit());
    assert(bottomHit.objectId == bottomNode->id());

    assert(topHit.objectId != bottomHit.objectId);

    std::cout << "MIR4D RAYPICKER: OK\n";
    return 0;
}
