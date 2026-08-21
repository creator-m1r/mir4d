#include "MirEngine/Viewport/ViewportController.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"

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

    // Pan happens in the camera view plane. With the Z-up convention the
    // orbit elevation (phi) is measured from +Z: at phi = pi/2 the eye sits on
    // the +Y axis looking down -Y, so the view plane is XZ (right = world -X,
    // up = world +Z). Panning therefore shifts the target in X and Z, leaving
    // Y fixed.
    camera.setOrbit(0.0, 3.14159265358979323846 * 0.5, 12.0);
    camera.setTarget({0.0, 0.0, 0.0});
    const auto topViewTarget = camera.target();
    controller.panBy(2.0, 1.0);
    const auto panned = camera.target();
    assert(std::abs(panned.y - topViewTarget.y) < 1e-9);
    assert(std::abs(panned.x - topViewTarget.x) > 1e-9);
    assert(std::abs(panned.z - topViewTarget.z) > 1e-9);

    // Zoom anchored at the screen center must keep the target fixed.
    camera.setOrbit(0.8, 1.2, 12.0);
    camera.setTarget({0.0, 0.0, 0.0});
    const auto centerTarget = camera.target();
    const auto centerDistance = camera.distance();
    const mir::PickRay ray = mir::RayPicker::buildRay(camera, 400.0, 300.0, 800, 600);
    assert(!ray.direction.isZero());
    controller.zoomAt(0.3, ray.origin, ray.direction);
    assert(camera.distance() < centerDistance);
    assert(std::abs(camera.target().x - centerTarget.x) < 1e-5);
    assert(std::abs(camera.target().y - centerTarget.y) < 1e-5);
    assert(std::abs(camera.target().z - centerTarget.z) < 1e-5);

    return 0;
}
