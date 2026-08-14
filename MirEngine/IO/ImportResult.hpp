#pragma once

#include "Format.hpp"
#include "../Math/Point.hpp"
#include "../Geometry/Tessellation/TriangleMesh.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace mir::io
{

struct ImportResult
{
    Format format{Format::Unknown};
    std::string sourcePath;
    std::string error;
    std::shared_ptr<TriangleMesh3> mesh;
    Point3 boundsMin{Point3::origin()};
    Point3 boundsMax{Point3::origin()};
    std::size_t triangleCount{0};

    [[nodiscard]] bool ok() const noexcept { return error.empty() && mesh != nullptr && mesh->isValid(); }
};

} // namespace mir::io
