#pragma once

#include "../Profile/Profile3.hpp"
#include "../Solid/Solid3.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace mir
{

/// Creates a triangular boundary representation by extruding a validated
/// planar profile along a direction.
class Extrude
{
public:
    static Solid3 create(
        const Profile3& profile,
        Scalar distance,
        const Direction3& direction)
    {
        if (!profile.isValid())
            throw std::invalid_argument("Extrude requires a valid planar profile");

        if (!std::isfinite(distance) || distance <= Scalar(0.0))
            throw std::invalid_argument("Extrude distance must be finite and positive");

        const Vector3 d = direction.asVector();
        if (d.isZero())
            throw std::invalid_argument("Extrude direction must be non-zero");

        const auto& curves = profile.outer().curves();
        if (curves.size() < 3)
            throw std::invalid_argument("Extrude requires at least three profile curves");

        std::vector<Point3> base;
        base.reserve(curves.size());
        for (const auto& curve : curves)
        {
            if (!curve)
                throw std::invalid_argument("Profile contains a null curve");
            base.push_back(curve->pointAt(curve->parameterStart()));
        }

        const Vector3 offset = d.normalized() * distance;
        std::vector<Point3> vertices = base;
        vertices.reserve(base.size() * 2);
        for (const Point3& p : base)
            vertices.push_back(p + offset);

        const std::size_t n = base.size();
        std::vector<Solid3::Triangle> triangles;
        triangles.reserve(n * 2 + (n - 2) * 2);

        for (std::size_t i = 1; i + 1 < n; ++i)
        {
            triangles.push_back({0, i + 1, i});
            triangles.push_back({n, n + i, n + i + 1});
        }

        for (std::size_t i = 0; i < n; ++i)
        {
            const std::size_t j = (i + 1) % n;
            triangles.push_back({i, j, n + j});
            triangles.push_back({i, n + j, n + i});
        }

        Solid3 solid(std::move(vertices), std::move(triangles));
        if (!solid.isValid())
            throw std::runtime_error("Extrude produced an invalid solid");

        return solid;
    }
};

} // namespace mir
