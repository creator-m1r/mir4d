#include "MirEngine/Viewport/ViewportController.hpp"

#include <cassert>
#include <cmath>

int main()
{
    mir::Camera camera;
    mir::ViewportController controller(&camera);

    const auto initialDistance = camera.distance();
    controller.zoom(0.5);
    assert(camera.distance() > 0.0);
    assert(std::abs(camera.distance() - initialDistance) > 1e-12);

    const auto initialTheta = camera.theta();
    controller.beginOrbit(0.0, 0.0);
    controller.move(10.0, 0.0);
    controller.end();
    assert(std::abs(camera.theta() - initialTheta) > 1e-12);

    const auto initialTarget = camera.target();
    controller.beginPan(0.0, 0.0);
    controller.move(5.0, -3.0);
    controller.end();
    const auto target = camera.target();
    assert(std::abs(target.x - initialTarget.x) > 1e-12 ||
           std::abs(target.y - initialTarget.y) > 1e-12);

    return 0;
}
