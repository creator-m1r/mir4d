#include "MirEngine/Geometry/Model/Model.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Viewport/ViewportRuntime.hpp"
#include <cassert>
#include <memory>
namespace {
class NullRenderer final : public MirEngine::Rendering::Renderer {
public:
    bool initialize() override { return true; }
    void render(mir::Scene& scene, MirEngine::Rendering::RenderContext& context) override { renderedScene=&scene; renderedWidth=context.viewportWidth; renderedHeight=context.viewportHeight; }
    void resize(std::uint32_t width, std::uint32_t height) override { resizedWidth=width; resizedHeight=height; }
    mir::Scene* renderedScene{nullptr}; std::uint32_t renderedWidth{0}; std::uint32_t renderedHeight{0}; std::uint32_t resizedWidth{0}; std::uint32_t resizedHeight{0};
}; }
int main()
{
    auto model = std::make_shared<mir::Model>(); mir::TriangleMesh3 mesh;
    mesh.vertices = {{-1.0,-1.0,0.0},{1.0,-1.0,0.0},{0.0,1.0,0.0}}; mesh.triangles={{0,1,2}}; model->setMesh(std::move(mesh));
    mir::Scene scene; const auto node=scene.createNode(std::move(model)); assert(node && scene.size()==1);
    NullRenderer renderer; mir::ViewportRuntime viewport(&renderer); viewport.resize(800,600); viewport.setScene(&scene); viewport.render();
    assert(renderer.renderedScene==&scene && renderer.renderedWidth==800 && renderer.renderedHeight==600);
    (void)viewport.pick(400.0,300.0); viewport.state().selection.select(node->id()); assert(viewport.state().selection.primary()==node->id());
    return 0;
}
