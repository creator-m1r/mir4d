// MirEngine/Tests/Render/HandGrabTest.cpp
// Vertical Slice v0.1 — Hand Grab (Preview → Commit → History).
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Viewport/ViewportRuntime.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Math/Transform.hpp"
#include <cassert>
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
    return mesh;
}

class NullRenderer final : public MirEngine::Rendering::Renderer
{
public:
    bool initialize() override { return true; }
    void render(mir::Scene&, MirEngine::Rendering::RenderContext&) override {}
    void resize(std::uint32_t, std::uint32_t) override {}
};
} // namespace

int main()
{
    auto model = std::make_shared<mir::Model>();
    model->setMesh(makeBox());

    mir::Scene scene;
    const auto node = scene.createNode(model);
    assert(node && scene.size() == 1);

    NullRenderer renderer;
    mir::ViewportRuntime viewport(&renderer);
    viewport.setScene(&scene);

    // ── World-ray pick hits the centered cube ───────────────────────────
    const auto hit = viewport.pickWorldRay(
        mir::Point3{0.0, 0.0, -5.0},
        mir::Vector3{0.0, 0.0, 1.0});
    assert(hit.hit());
    assert(hit.objectId == node->id());

    // ── Grab: preview mutates the scene, but NOT history ────────────────
    viewport.beginHandGrab(node->id());
    assert(viewport.isHandGrabbing());

    const mir4d::Transform start = node->transform();
    mir4d::Transform moved = start;
    moved.position = {10.0, 2.0, -3.0};
    viewport.previewHandGrab(moved);
    assert(node->transform() == moved);
    assert(!viewport.canUndo());           // preview must not enter history
    assert(viewport.state().selection.primary() == node->id());

    // A second preview frame must also stay out of history.
    mir4d::Transform moved2 = moved;
    moved2.position = {12.0, 2.0, -3.0};
    viewport.previewHandGrab(moved2);
    assert(node->transform() == moved2);
    assert(!viewport.canUndo());           // still exactly zero history entries

    // ── Commit → exactly one history entry ─────────────────────────────
    viewport.commitHandGrab();
    assert(!viewport.isHandGrabbing());
    assert(viewport.canUndo());
    assert(!viewport.canRedo());
    assert(node->transform() == moved2);

    // ── Undo restores the snapshot ─────────────────────────────────────
    assert(viewport.undo());
    assert(node->transform() == start);

    // ── Redo reapplies the committed move ──────────────────────────────
    assert(viewport.redo());
    assert(node->transform() == moved2);

    // ── Cancel restores the grab snapshot and adds no history ──────────
    // After redo() the object sits at moved2, so that is the snapshot the
    // next grab must restore to on cancel.
    viewport.beginHandGrab(node->id());
    mir4d::Transform moved3 = start;
    moved3.position = {99.0, 0.0, 0.0};
    viewport.previewHandGrab(moved3);
    assert(node->transform() == moved3);
    viewport.cancelHandGrab();
    assert(node->transform() == moved2);   // grab snapshot, not the original start
    assert(viewport.canUndo());            // committed command is untouched
    assert(!viewport.canRedo());           // cancel added no new entry

    std::cout << "MIR4D HANDGRAB: OK\n";
    return 0;
}
