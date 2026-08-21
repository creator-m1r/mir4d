#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace mir
{

class BRepGeometryStore
{
public:
    struct Checkpoint
    {
        std::size_t pointCount{0};
        std::size_t curveCount{0};
        std::size_t surfaceCount{0};
    };

    [[nodiscard]] Checkpoint checkpoint() const noexcept
    {
        return {points_.size(), curves_.size(), surfaces_.size()};
    }

    void rollback(Checkpoint checkpoint) noexcept
    {
        if (checkpoint.pointCount <= points_.size())
            points_.resize(checkpoint.pointCount);
        if (checkpoint.curveCount <= curves_.size())
            curves_.resize(checkpoint.curveCount);
        if (checkpoint.surfaceCount <= surfaces_.size())
            surfaces_.resize(checkpoint.surfaceCount);
    }

    [[nodiscard]] BRepPointHandle addPoint(BRepPointGeometry geometry)
    {
        const BRepIndex index = static_cast<BRepIndex>(points_.size());
        points_.push_back(std::move(geometry));
        return BRepPointHandle{index};
    }

    [[nodiscard]] BRepCurveHandle addCurve(BRepCurveGeometry geometry)
    {
        const BRepIndex index = static_cast<BRepIndex>(curves_.size());
        curves_.push_back(std::move(geometry));
        return BRepCurveHandle{index};
    }

    [[nodiscard]] BRepSurfaceHandle addSurface(BRepSurfaceGeometry geometry)
    {
        const BRepIndex index = static_cast<BRepIndex>(surfaces_.size());
        surfaces_.push_back(std::move(geometry));
        return BRepSurfaceHandle{index};
    }

    [[nodiscard]] const BRepPointGeometry* point(BRepPointHandle handle) const noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < points_.size() ? &points_[idx] : nullptr;
    }

    [[nodiscard]] BRepPointGeometry* point(BRepPointHandle handle) noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < points_.size() ? &points_[idx] : nullptr;
    }

    [[nodiscard]] const BRepCurveGeometry* curve(BRepCurveHandle handle) const noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < curves_.size() ? &curves_[idx] : nullptr;
    }

    [[nodiscard]] BRepCurveGeometry* curve(BRepCurveHandle handle) noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < curves_.size() ? &curves_[idx] : nullptr;
    }

    [[nodiscard]] const BRepSurfaceGeometry* surface(BRepSurfaceHandle handle) const noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < surfaces_.size() ? &surfaces_[idx] : nullptr;
    }

    [[nodiscard]] BRepSurfaceGeometry* surface(BRepSurfaceHandle handle) noexcept
    {
        if (!handle.valid()) return nullptr;
        const std::size_t idx = static_cast<std::size_t>(handle.index);
        return idx < surfaces_.size() ? &surfaces_[idx] : nullptr;
    }

    [[nodiscard]] const std::vector<BRepPointGeometry>& points() const noexcept { return points_; }
    [[nodiscard]] const std::vector<BRepCurveGeometry>& curves() const noexcept { return curves_; }
    [[nodiscard]] const std::vector<BRepSurfaceGeometry>& surfaces() const noexcept { return surfaces_; }

    [[nodiscard]] std::size_t pointCount() const noexcept { return points_.size(); }
    [[nodiscard]] std::size_t curveCount() const noexcept { return curves_.size(); }
    [[nodiscard]] std::size_t surfaceCount() const noexcept { return surfaces_.size(); }

    void clear() noexcept
    {
        points_.clear();
        curves_.clear();
        surfaces_.clear();
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        for (const BRepPointGeometry& item : points_) if (!item.isFinite()) return false;
        for (const BRepCurveGeometry& item : curves_) if (!item.isValid()) return false;
        for (const BRepSurfaceGeometry& item : surfaces_) if (!item.isValid()) return false;
        return true;
    }

private:
    std::vector<BRepPointGeometry> points_;
    std::vector<BRepCurveGeometry> curves_;
    std::vector<BRepSurfaceGeometry> surfaces_;
};

}