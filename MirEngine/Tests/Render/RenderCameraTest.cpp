#include "MirEngine/Render/RenderCamera.hpp"

#include <cassert>
#include <cmath>

namespace
{

bool finiteMatrix(const mir::RenderMat4& matrix)
{
    for (double value : matrix.m)
        if (!std::isfinite(value))
            return false;
    return true;
}

} // namespace

int main()
{
    mir::RenderCamera camera;
    camera.setTarget({1.0, 2.0, 3.0});
    camera.setDistance(8.0);
    camera.setYaw(0.4);
    camera.setPitch(0.3);
    camera.setPerspective(1.0, 16.0 / 9.0, 0.1, 1000.0);

    const auto position = camera.position();
    assert(std::isfinite(position.x));
    assert(std::isfinite(position.y));
    assert(std::isfinite(position.z));
    assert(finiteMatrix(camera.viewMatrix()));
    assert(finiteMatrix(camera.projectionMatrix()));

    camera.setDistance(-1.0);
    assert(camera.distance() > 0.0);
    camera.setPitch(100.0);
    assert(camera.pitch() < 1.56);

    return 0;
}
