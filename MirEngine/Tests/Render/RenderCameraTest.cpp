#include "MirEngine/Viewport/Camera.hpp"
#include "MirEngine/Math/TransformMatrix.hpp"

#include <cassert>
#include <cmath>

namespace
{

bool finiteMatrix(const mir::Matrix4& matrix)
{
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            if (!std::isfinite(static_cast<double>(matrix(row, column))))
                return false;
    return true;
}

}

int main()
{
    mir::Camera camera;
    camera.setTarget({1.0, 2.0, 3.0});
    camera.setOrbit(0.4, 0.3, 8.0);
    camera.setPerspective(1.0, 16.0 / 9.0, 0.1, 1000.0);

    const auto position = camera.position();
    assert(std::isfinite(position.x));
    assert(std::isfinite(position.y));
    assert(std::isfinite(position.z));
    assert(finiteMatrix(camera.viewMatrix()));
    assert(finiteMatrix(camera.projectionMatrix()));

    camera.setOrbit(0.4, 0.3, -1.0);
    assert(camera.distance() > 0.0);

    camera.setOrbit(0.4, 100.0, 8.0);
    assert(camera.phi() < 3.1416);
    assert(camera.phi() > 0.0);

    camera.setProjection(mir::CameraProjection::Orthographic);
    assert(finiteMatrix(camera.projectionMatrix()));

    return 0;
}