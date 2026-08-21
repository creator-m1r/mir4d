
#include "MirEngine/Geometry/Query/Query.hpp"

#include <cassert>
#include <cmath>

namespace
{

[[nodiscard]] bool near(const mir::Point3& a, const mir::Point3& b, mir::Scalar tolerance = 1e-9)
{
    return a.squaredDistance(b) <= tolerance * tolerance;
}

[[nodiscard]] bool near(mir::Scalar a, mir::Scalar b, mir::Scalar tolerance = 1e-9)
{
    return std::abs(a - b) <= tolerance;
}

}

int main()
{

    {
        const mir::Segment3 segment({0.0, 0.0, 0.0}, {2.0, 0.0, 0.0});
        assert(near(segment.length(), 2.0));
        assert(near(segment.midpoint(), {1.0, 0.0, 0.0}));
        assert(near(segment.pointAt(0.25), {0.5, 0.0, 0.0}));

        const mir::Line3 line({1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
        assert(line.isValid());
        assert(near(line.pointAt(2.0), {1.0, 2.0, 0.0}));

        const mir::Ray3 ray({0.0, 0.0, 0.0}, {0.0, 0.0, -1.0});
        assert(ray.isValid());
        assert(near(ray.pointAt(5.0), {0.0, 0.0, -5.0}));
    }

    {
        const mir::Line3 axisX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::projectPointOnLine({0.0, 2.0, 0.0}, axisX), {0.0, 0.0, 0.0}));

        const mir::Ray3 rayX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::projectPointOnRay({-1.0, 2.0, 0.0}, rayX), {0.0, 0.0, 0.0}));
        assert(near(mir::GeometryQuery::projectPointOnRay({3.0, 2.0, 0.0}, rayX), {3.0, 0.0, 0.0}));

        const mir::Segment3 segX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::projectPointOnSegment({3.0, 0.0, 0.0}, segX), {1.0, 0.0, 0.0}));
        assert(near(mir::GeometryQuery::projectPointOnSegment({-3.0, 0.0, 0.0}, segX), {0.0, 0.0, 0.0}));
        assert(near(mir::GeometryQuery::projectPointOnSegment({0.5, 4.0, 0.0}, segX), {0.5, 0.0, 0.0}));
    }

    {
        const mir::Line3 axisX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::distancePointToLine({0.0, 1.0, 0.0}, axisX), 1.0));

        const mir::Ray3 rayX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::distancePointToRay({-1.0, 1.0, 0.0}, rayX), std::sqrt(2.0)));
        assert(near(mir::GeometryQuery::distancePointToRay({2.0, 1.0, 0.0}, rayX), 1.0));

        const mir::Segment3 segX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::distancePointToSegment({3.0, 0.0, 0.0}, segX), 2.0));
        assert(near(mir::GeometryQuery::distancePointToSegment({0.5, 3.0, 0.0}, segX), 3.0));
    }

    {

        const mir::Line3 axisX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Line3 zLine({0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});
        assert(near(mir::GeometryQuery::distanceLineToLine(axisX, zLine), 1.0));

        const mir::Line3 yOffset({0.0, 2.0, 0.0}, {1.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::distanceLineToLine(axisX, yOffset), 2.0));

        const mir::Segment3 segA({0.0, 0.0, 0.0}, {2.0, 0.0, 0.0});
        const mir::Segment3 segB({1.0, -1.0, 0.0}, {1.0, 1.0, 0.0});
        assert(near(mir::GeometryQuery::distanceSegmentToSegment(segA, segB), 0.0));

        const mir::Segment3 segC({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Segment3 segD({2.0, 1.0, 0.0}, {2.0, 2.0, 0.0});
        assert(near(mir::GeometryQuery::distanceSegmentToSegment(segC, segD), std::sqrt(2.0)));

        const mir::Segment3 segE({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Segment3 segF({2.0, 0.0, 0.0}, {3.0, 0.0, 0.0});
        assert(near(mir::GeometryQuery::distanceSegmentToSegment(segE, segF), 1.0));
    }

    {
        const mir::MathPlane plane({0.0, 0.0, 1.0}, {0.0, 0.0, 0.0});

        const mir::Line3 line({0.0, 0.0, 1.0}, {0.0, 0.0, -1.0});
        const auto hit = mir::GeometryQuery::intersectLinePlane(line, plane);
        assert(hit.has_value());
        assert(near(*hit, {0.0, 0.0, 0.0}));

        const mir::Line3 parallelLine({0.0, 0.0, 1.0}, {1.0, 0.0, 0.0});
        assert(!mir::GeometryQuery::intersectLinePlane(parallelLine, plane).has_value());

        const mir::Ray3 awayRay({0.0, 0.0, -1.0}, {0.0, 0.0, -1.0});
        assert(!mir::GeometryQuery::intersectRayPlane(awayRay, plane).has_value());

        const mir::Ray3 towardRay({0.0, 0.0, 1.0}, {0.0, 0.0, -1.0});
        const auto rayHit = mir::GeometryQuery::intersectRayPlane(towardRay, plane);
        assert(rayHit.has_value());
        assert(near(*rayHit, {0.0, 0.0, 0.0}));

        const mir::Segment3 crossing({0.0, 0.0, 1.0}, {0.0, 0.0, -1.0});
        const auto segHit = mir::GeometryQuery::intersectSegmentPlane(crossing, plane);
        assert(segHit.has_value());
        assert(near(*segHit, {0.0, 0.0, 0.0}));

        const mir::Segment3 above({0.0, 0.0, 1.0}, {0.0, 0.0, 2.0});
        assert(!mir::GeometryQuery::intersectSegmentPlane(above, plane).has_value());
    }

    {

        const mir::Line3 axisX({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Line3 yLine({1.0, -1.0, 0.0}, {0.0, 1.0, 0.0});
        const auto hit = mir::GeometryQuery::intersectLineLine(axisX, yLine);
        assert(hit.has_value());
        assert(near(*hit, {1.0, 0.0, 0.0}));

        const mir::Line3 zLine({0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});
        assert(!mir::GeometryQuery::intersectLineLine(axisX, zLine).has_value());

        const mir::Line3 yOffset({0.0, 2.0, 0.0}, {1.0, 0.0, 0.0});
        assert(!mir::GeometryQuery::intersectLineLine(axisX, yOffset).has_value());

        const mir::Segment3 segA({0.0, 0.0, 0.0}, {2.0, 0.0, 0.0});
        const mir::Segment3 segB({1.0, -1.0, 0.0}, {1.0, 1.0, 0.0});
        const auto segHit = mir::GeometryQuery::intersectSegmentSegment(segA, segB);
        assert(segHit.has_value());
        assert(near(*segHit, {1.0, 0.0, 0.0}));

        const mir::Segment3 segC({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Segment3 segD({2.0, 0.0, 0.0}, {3.0, 0.0, 0.0});
        assert(!mir::GeometryQuery::intersectSegmentSegment(segC, segD).has_value());

        const mir::Segment3 segE({0.0, 0.0, 0.0}, {2.0, 0.0, 0.0});
        const mir::Segment3 segF({1.0, 0.0, 0.0}, {3.0, 0.0, 0.0});
        const auto overlap = mir::GeometryQuery::intersectSegmentSegment(segE, segF);
        assert(overlap.has_value());
        assert(near(*overlap, {1.0, 0.0, 0.0}));

        const mir::Segment3 segG({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0});
        const mir::Segment3 segH({0.0, 1.0, 0.0}, {1.0, 1.0, 0.0});
        assert(!mir::GeometryQuery::intersectSegmentSegment(segG, segH).has_value());
    }

    {
        const mir::Point3 v0{0.0, 0.0, 0.0};
        const mir::Point3 v1{1.0, 0.0, 0.0};
        const mir::Point3 v2{0.0, 1.0, 0.0};

        const mir::Ray3 ray({0.2, 0.2, 1.0}, {0.0, 0.0, -1.0});
        const auto hit = mir::GeometryQuery::intersectRayTriangle(ray, v0, v1, v2);
        assert(hit.has_value());
        assert(near(*hit, {0.2, 0.2, 0.0}));

        const mir::Ray3 miss({2.0, 2.0, 1.0}, {0.0, 0.0, -1.0});
        assert(!mir::GeometryQuery::intersectRayTriangle(miss, v0, v1, v2).has_value());

        const mir::Ray3 away({0.2, 0.2, -1.0}, {0.0, 0.0, -1.0});
        assert(!mir::GeometryQuery::intersectRayTriangle(away, v0, v1, v2).has_value());

        const mir::Ray3 parallel({0.2, 0.2, 1.0}, {1.0, 0.0, 0.0});
        assert(!mir::GeometryQuery::intersectRayTriangle(parallel, v0, v1, v2).has_value());
    }

    return 0;
}