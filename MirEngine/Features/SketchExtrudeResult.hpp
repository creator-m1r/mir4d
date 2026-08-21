#pragma once

#include "SketchExtrudeGeometry.hpp"
#include "../Sketch/SketchSolidProfile.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

struct SketchExtrudeEdge
{
    std::uint32_t start{0};
    std::uint32_t end{0};
};

struct SketchExtrudeFace
{
    std::vector<std::uint32_t> vertices;
    bool cap{false};
    bool side{false};
};

struct SketchExtrudeResult
{
    std::vector<SketchExtrudePoint3D> vertices;
    std::vector<SketchExtrudeEdge> edges;
    std::vector<SketchExtrudeFace> faces;
    bool valid{false};

    [[nodiscard]] bool isUsableForBRep() const noexcept
    {
        return valid && !vertices.empty() && !faces.empty();
    }
};

}
