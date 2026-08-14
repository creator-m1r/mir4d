#include "MirEngine/Viewport/ViewportState.hpp"
#include <cassert>
int main()
{
    mir::ViewportState viewport;
    viewport.resize(800, 600);
    assert(viewport.width == 800 && viewport.height == 600);
    assert(viewport.camera.distance() > 0.0);
    assert(viewport.selection.empty());
    viewport.selection.select(42);
    assert(viewport.selection.contains(42) && viewport.selection.primary() == 42);
    const auto thetaBefore = viewport.camera.theta();
    viewport.controller.beginOrbit(100.0, 100.0);
    viewport.controller.move(120.0, 90.0);
    viewport.controller.end();
    assert(viewport.camera.theta() != thetaBefore);
    viewport.controller.zoom(0.1);
    assert(viewport.camera.distance() > 0.0);
    return 0;
}
