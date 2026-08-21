#pragma once

namespace mir
{

struct SketchExtrudePoint3D
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct SketchExtrudeVector3D
{
    double x{0.0};
    double y{0.0};
    double z{1.0};

    [[nodiscard]] double lengthSquared() const noexcept
    {
        return x * x + y * y + z * z;
    }
};

}
