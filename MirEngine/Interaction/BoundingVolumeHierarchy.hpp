#pragma once

#include "../Math/Point.hpp"
#include "../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace mir
{

/// Axis-aligned bounding box used as the BVH primitive envelope.
struct BVHAABB
{
    Point3 min{};
    Point3 max{};
};

/// Bounding volume hierarchy over primitive AABBs, queried by ray.
///
/// The tree is built once in a stable local space and reused across picks;
/// the caller transforms the picking ray into that same space before querying,
/// so the hierarchy never has to be rebuilt when the owning node merely moves
/// or rotates. Only a change of geometry invalidates the tree.
///
/// `queryRay` returns every primitive whose AABB, inflated by `radius`,
/// intersects the ray. It is intentionally conservative: the precise
/// (distance-based) acceptance test is expected to run on the returned
/// candidates afterwards, so over-inclusion is harmless and under-inclusion
/// is avoided by construction.
class BoundingVolumeHierarchy
{
public:
    void build(const std::vector<BVHAABB>& boxes,
               const std::vector<std::size_t>& indices)
    {
        boxes_ = boxes;
        indices_ = indices;
        nodes_.clear();
        if (!indices_.empty())
            buildRange(0, indices_.size());
    }

    /// Convenience builder for point clouds. The returned primitive index
    /// equals the point's position in `points`.
    void buildPoints(const std::vector<Point3>& points)
    {
        std::vector<BVHAABB> boxes(points.size());
        std::vector<std::size_t> indices(points.size());
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            boxes[i].min = boxes[i].max = points[i];
            indices[i] = i;
        }
        build(boxes, indices);
    }

    [[nodiscard]] bool empty() const noexcept { return nodes_.empty(); }

    /// Collects primitive indices whose radius-inflated AABB intersects the
    /// ray `origin + t*dir` (dir need not be normalized).
    void queryRay(const Point3& origin,
                  const Vector3& dir,
                  Scalar radius,
                  std::vector<std::size_t>& out) const
    {
        if (nodes_.empty() || dir.isZero())
            return;
        const Vector3 invDir{
            safeInv(dir.x), safeInv(dir.y), safeInv(dir.z)};
        queryRange(0, origin, invDir, radius, out);
    }

private:
    struct Node
    {
        BVHAABB box{};
        std::size_t start{0};
        std::size_t count{0};
        int left{-1};
        int right{-1};
    };

    std::vector<Node> nodes_;
    std::vector<BVHAABB> boxes_;
    std::vector<std::size_t> indices_;

    static Scalar safeInv(Scalar d) noexcept
    {
        constexpr Scalar e = static_cast<Scalar>(1e-12);
        if (std::abs(d) < e)
            return d >= Scalar(0) ? std::numeric_limits<Scalar>::max()
                                  : -std::numeric_limits<Scalar>::max();
        return Scalar(1) / d;
    }

    static Point3 centroid(const BVHAABB& b) noexcept
    {
        return Point3{
            (b.min.x + b.max.x) * Scalar(0.5),
            (b.min.y + b.max.y) * Scalar(0.5),
            (b.min.z + b.max.z) * Scalar(0.5)};
    }

    static Scalar comp(const Point3& p, int axis) noexcept
    {
        return axis == 0 ? p.x : (axis == 1 ? p.y : p.z);
    }

    static Scalar comp(const Vector3& v, int axis) noexcept
    {
        return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
    }

    static int longestAxis(const Vector3& d) noexcept
    {
        if (d.x >= d.y && d.x >= d.z) return 0;
        if (d.y >= d.z) return 1;
        return 2;
    }

    int buildRange(std::size_t start, std::size_t count)
    {
        const int idx = static_cast<int>(nodes_.size());
        nodes_.emplace_back();

        BVHAABB bound = boxes_[indices_[start]];
        Point3 cmin = centroid(bound);
        Point3 cmax = cmin;
        for (std::size_t i = start; i < start + count; ++i)
        {
            const BVHAABB& b = boxes_[indices_[i]];
            bound.min.x = std::min(bound.min.x, b.min.x);
            bound.min.y = std::min(bound.min.y, b.min.y);
            bound.min.z = std::min(bound.min.z, b.min.z);
            bound.max.x = std::max(bound.max.x, b.max.x);
            bound.max.y = std::max(bound.max.y, b.max.y);
            bound.max.z = std::max(bound.max.z, b.max.z);
            const Point3 c = centroid(b);
            cmin.x = std::min(cmin.x, c.x);
            cmin.y = std::min(cmin.y, c.y);
            cmin.z = std::min(cmin.z, c.z);
            cmax.x = std::max(cmax.x, c.x);
            cmax.y = std::max(cmax.y, c.y);
            cmax.z = std::max(cmax.z, c.z);
        }

        constexpr std::size_t kLeafThreshold = 8;
        if (count <= kLeafThreshold)
        {
            nodes_[idx] = Node{bound, start, count, -1, -1};
            return idx;
        }

        const int axis = longestAxis(cmax - cmin);
        const Scalar split = (comp(cmin, axis) + comp(cmax, axis)) * Scalar(0.5);

        auto begin = indices_.begin() + static_cast<std::ptrdiff_t>(start);
        auto end = begin + static_cast<std::ptrdiff_t>(count);
        auto midIt = std::partition(
            begin, end, [&](std::size_t id) {
                return comp(centroid(boxes_[id]), axis) <= split;
            });
        std::size_t midCount =
            static_cast<std::size_t>(midIt - begin);
        if (midCount == 0 || midCount == count)
            midCount = count / 2;

        const int left = buildRange(start, midCount);
        const int right = buildRange(start + midCount, count - midCount);
        nodes_[idx] = Node{bound, start, count, left, right};
        return idx;
    }

    void queryRange(int node,
                    const Point3& o,
                    const Vector3& invDir,
                    Scalar radius,
                    std::vector<std::size_t>& out) const
    {
        if (node < 0)
            return;
        const Node& n = nodes_[static_cast<std::size_t>(node)];
        if (!intersects(n.box, o, invDir, radius))
            return;
        if (n.left < 0)
        {
            for (std::size_t i = n.start; i < n.start + n.count; ++i)
            {
                const std::size_t id = indices_[i];
                if (intersects(boxes_[id], o, invDir, radius))
                    out.push_back(id);
            }
            return;
        }
        queryRange(n.left, o, invDir, radius, out);
        queryRange(n.right, o, invDir, radius, out);
    }

    static bool intersects(const BVHAABB& b,
                           const Point3& o,
                           const Vector3& invDir,
                           Scalar radius) noexcept
    {
        const Scalar mn[3] = {b.min.x - radius, b.min.y - radius, b.min.z - radius};
        const Scalar mx[3] = {b.max.x + radius, b.max.y + radius, b.max.z + radius};
        Scalar t0 = -std::numeric_limits<Scalar>::max();
        Scalar t1 = std::numeric_limits<Scalar>::max();
        const Scalar org[3] = {o.x, o.y, o.z};
        const Scalar iv[3] = {invDir.x, invDir.y, invDir.z};
        for (int i = 0; i < 3; ++i)
        {
            const Scalar tNear = (mn[i] - org[i]) * iv[i];
            const Scalar tFar = (mx[i] - org[i]) * iv[i];
            const Scalar lo = std::min(tNear, tFar);
            const Scalar hi = std::max(tNear, tFar);
            t0 = std::max(t0, lo);
            t1 = std::min(t1, hi);
            if (t0 > t1)
                return false;
        }
        return t1 >= std::max(Scalar(0), t0);
    }
};

} // namespace mir
