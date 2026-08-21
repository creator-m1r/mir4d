
#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Viewport/ViewportRuntime.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <cassert>
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
    return mesh;
}

struct CaptureRenderer final : public MirEngine::Rendering::Renderer
{
    MirEngine::Rendering::HandSkeletonRenderData captured{};
    bool initialize() override { return true; }
    void render(mir::Scene&, MirEngine::Rendering::RenderContext& ctx) override
    {
        captured = ctx.handSkeleton;
    }
    void resize(std::uint32_t, std::uint32_t) override {}
};
}

int main()
{
    auto model = std::make_shared<mir::Model>();
    model->setMesh(makeBox());

    mir::Scene scene;
    scene.createNode(model);

    CaptureRenderer renderer;
    mir::ViewportRuntime viewport(&renderer);
    viewport.setScene(&scene);

    constexpr int kJointCount = 21;
    MirEngine::Rendering::HandSkeletonRenderData data{};
    data.mode = 2;
    data.handCount = 1;
    for (int j = 0; j < kJointCount; ++j)
    {
        data.positions[0][j * 3 + 0] = static_cast<float>(j);
        data.positions[0][j * 3 + 1] = 0.0f;
        data.positions[0][j * 3 + 2] = 0.0f;
        data.confidence[0][j] = 1.0f;
    }
    data.handedness[0] = 1;
    data.pinch[0] = 0.8f;

    viewport.setHandSkeleton(data);
    viewport.render();
    assert(renderer.captured.handCount == 1);
    assert(renderer.captured.mode == 2);
    assert(renderer.captured.handedness[0] == 1);
    assert(renderer.captured.pinch[0] == 0.8f);
    assert(renderer.captured.positions[0][0] == 0.0f);
    assert(renderer.captured.positions[0][20 * 3] == 20.0f);

    viewport.clearHandSkeleton();
    viewport.render();
    assert(renderer.captured.handCount == 0);
    assert(renderer.captured.mode == 0);

    std::cout << "MIR4D HANDSKELETON: OK\n";
    return 0;
}
