#pragma once

// MirEngine/BRep/Core/BRepModel.hpp
// Canonical owning model for B-Rep data and root solids.

#include "MirEngine/BRep/Core/BRepHandles.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/BRep/Topology/BRepTopologyStore.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace mir
{

class BRepModel
{
public:
    struct Checkpoint
    {
        BRepGeometryStore::Checkpoint geometry;
        BRepTopologyStore::Checkpoint topology;
        std::size_t rootSolidCount{0};
    };

    [[nodiscard]] Checkpoint checkpoint() const noexcept
    {
        return {geometry_.checkpoint(), topology_.checkpoint(), roots_.size()};
    }

    // Transactional rollback for builder operations. Only data appended after
    // the checkpoint is removed; pre-existing model handles remain intact.
    void rollback(Checkpoint checkpoint) noexcept
    {
        geometry_.rollback(checkpoint.geometry);
        topology_.rollback(checkpoint.topology);
        if (checkpoint.rootSolidCount <= roots_.size())
            roots_.resize(checkpoint.rootSolidCount);
    }

    [[nodiscard]] BRepGeometryStore& geometry() noexcept { return geometry_; }
    [[nodiscard]] const BRepGeometryStore& geometry() const noexcept { return geometry_; }

    [[nodiscard]] BRepTopologyStore& topology() noexcept { return topology_; }
    [[nodiscard]] const BRepTopologyStore& topology() const noexcept { return topology_; }

    /// Adds a solid to the model's root set exactly once.
    /// Root membership is non-owning; the topology store owns the solid record.
    void addRootSolid(BRepSolidHandle solid)
    {
        if (!solid.valid() || !topology_.solid(solid))
            return;

        if (std::find(roots_.begin(), roots_.end(), solid) == roots_.end())
            roots_.push_back(solid);
    }

    [[nodiscard]] bool containsRootSolid(BRepSolidHandle solid) const noexcept
    {
        return solid.valid() &&
               std::find(roots_.begin(), roots_.end(), solid) != roots_.end();
    }

    [[nodiscard]] const std::vector<BRepSolidHandle>& rootSolids() const noexcept
    {
        return roots_;
    }

    void clear() noexcept
    {
        geometry_.clear();
        topology_.clear();
        roots_.clear();
    }

    /// True only when the complete B-Rep model contains no geometry,
    /// topology, or root references.
    [[nodiscard]] bool empty() const noexcept
    {
        return roots_.empty() &&
               topology_.vertexCount() == 0 &&
               topology_.edgeCount() == 0 &&
               topology_.wireCount() == 0 &&
               topology_.faceCount() == 0 &&
               topology_.shellCount() == 0 &&
               topology_.solidCount() == 0 &&
               geometry_.pointCount() == 0 &&
               geometry_.curveCount() == 0 &&
               geometry_.surfaceCount() == 0;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!geometry_.isValid())
            return false;

        for (BRepSolidHandle solidHandle : roots_)
        {
            const BRepSolid* solid = topology_.solid(solidHandle);
            if (!solid || !solid->isValid())
                return false;
        }
        return true;
    }

private:
    BRepGeometryStore geometry_;
    BRepTopologyStore topology_;
    std::vector<BRepSolidHandle> roots_;
};

} // namespace mir