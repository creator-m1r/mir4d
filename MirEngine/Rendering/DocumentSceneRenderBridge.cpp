#include "DocumentSceneRenderBridge.hpp"

#include "../Geometry/Model/Model.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace mir::rendering
{
namespace
{

void includePoint(DocumentSceneRenderBridge::Bounds& bounds, const Point3& point)
{
    if (!bounds.valid)
    {
        bounds.min = point;
        bounds.max = point;
        bounds.valid = true;
        return;
    }

    bounds.min.x = std::min(bounds.min.x, point.x);
    bounds.min.y = std::min(bounds.min.y, point.y);
    bounds.min.z = std::min(bounds.min.z, point.z);
    bounds.max.x = std::max(bounds.max.x, point.x);
    bounds.max.y = std::max(bounds.max.y, point.y);
    bounds.max.z = std::max(bounds.max.z, point.z);
}

void includeNodeBounds(DocumentSceneRenderBridge::Bounds& bounds,
                       const ModelNode& node)
{
    if (!node.model() || !node.model()->hasMesh())
        return;

    const Point3 localMin = node.model()->boundsMin();
    const Point3 localMax = node.model()->boundsMax();

    const std::array<Point3, 8> corners = {
        Point3{localMin.x, localMin.y, localMin.z},
        Point3{localMax.x, localMin.y, localMin.z},
        Point3{localMin.x, localMax.y, localMin.z},
        Point3{localMax.x, localMax.y, localMin.z},
        Point3{localMin.x, localMin.y, localMax.z},
        Point3{localMax.x, localMin.y, localMax.z},
        Point3{localMin.x, localMax.y, localMax.z},
        Point3{localMax.x, localMax.y, localMax.z}};

    const Transform& transform = node.transform();
    for (const Point3& corner : corners)
        includePoint(bounds, transform.transformPoint(corner));
}

}

double DocumentSceneRenderBridge::Bounds::radius() const noexcept
{
    if (!valid)
        return 0.0;

    const double dx = max.x - min.x;
    const double dy = max.y - min.y;
    const double dz = max.z - min.z;
    return 0.5 * std::sqrt(dx * dx + dy * dy + dz * dz);
}

DocumentSceneRenderBridge::Bounds DocumentSceneRenderBridge::rebuild(
    const Scene& documentScene)
{
    const std::uint64_t revision = documentScene.contentRevision();
    if (sourceRevision_ == revision)
        return cachedBounds_;

    cachedBounds_ = {};
    objectIds_.clear();
    sourceRevision_ = revision;

    for (const auto& node : documentScene.nodes())
    {
        if (!node || !node->isValid())
            continue;

        objectIds_.push_back(node->id());
        includeNodeBounds(cachedBounds_, *node);
    }

    return cachedBounds_;
}

}
