// MirEngine/Tests/Sketch/SketchDocumentSolverTest.cpp
// Tests for the universal sketch constraint solver (SketchDocumentSolver).

#include "MirEngine/Sketch/SketchDocument.hpp"
#include "MirEngine/Sketch/SketchDocumentSolver.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <variant>

namespace
{

constexpr double kEps = 1.0e-6;

void checkNear(double actual, double expected, double eps, const char* label)
{
    if (std::fabs(actual - expected) > eps)
    {
        std::cerr << "[FAIL] " << label << ": actual=" << actual
                  << " expected=" << expected << " eps=" << eps << "\n";
        std::exit(1);
    }
}

double lineLength(const mir::SketchLine2D& l)
{
    return std::hypot(l.end.x - l.start.x, l.end.y - l.start.y);
}

double pointToLineDistance(double px, double py, const mir::SketchLine2D& l)
{
    const double dx = l.end.x - l.start.x;
    const double dy = l.end.y - l.start.y;
    const double len = std::hypot(dx, dy);
    if (len < 1.0e-9)
        return 0.0;
    return std::fabs((px - l.start.x) * dy - (py - l.start.y) * dx) / len;
}

const mir::SketchLine2D& getLine(const mir::SketchGeometryStore& g, std::uint32_t id)
{
    const auto* p = g.find(id);
    assert(p);
    return std::get<mir::SketchLine2D>(*p);
}

const mir::SketchCircle2D& getCircle(const mir::SketchGeometryStore& g, std::uint32_t id)
{
    const auto* p = g.find(id);
    assert(p);
    return std::get<mir::SketchCircle2D>(*p);
}

} // namespace

int main()
{
    // Scenario 1: linked contour (A bottom, B right, D left).
    {
        mir::SketchDocument doc;
        auto& g = doc.geometry();
        auto& c = doc.constraints();

        const auto a = g.addLine({0.0, 0.0}, {4.0, 0.0});
        const auto b = g.addLine({4.0, 0.0}, {4.0, 5.0});
        const auto d = g.addLine({0.0, 5.0}, {0.0, 0.0});

        c.add(mir::SketchConstraintType::Horizontal, a);
        c.add(mir::SketchConstraintType::Vertical, b);
        c.add(mir::SketchConstraintType::Vertical, d);
        c.add(mir::SketchConstraintType::Distance, a, 0, 4.0);
        c.add(mir::SketchConstraintType::Equal, a, b);
        c.add(mir::SketchConstraintType::Equal, a, d);
        c.add(mir::SketchConstraintType::Parallel, b, d);
        c.add(mir::SketchConstraintType::Perpendicular, a, b);
        c.add(mir::SketchConstraintType::Coincident, a, b);
        c.add(mir::SketchConstraintType::Coincident, d, a);
        c.add(mir::SketchConstraintType::Angle, a, b, 90.0 * M_PI / 180.0);

        const auto result = mir::SketchDocumentSolver::solve(doc);
        assert(result.converged);

        const auto la = getLine(g, a);
        const auto lb = getLine(g, b);
        const auto ld = getLine(g, d);

        checkNear(lineLength(la), 4.0, 1.0e-4, "A length");
        checkNear(lineLength(lb), 4.0, 1.0e-4, "B length (equalized)");
        checkNear(lineLength(ld), 4.0, 1.0e-4, "D length (equalized)");
        checkNear(la.start.y, la.end.y, 1.0e-4, "A horizontal");
        checkNear(lb.start.x, lb.end.x, 1.0e-4, "B vertical");
        checkNear(ld.start.x, ld.end.x, 1.0e-4, "D vertical");
        checkNear(lb.start.x, la.end.x, 1.0e-4, "B.start == A.end");
        checkNear(lb.start.y, la.end.y, 1.0e-4, "B.start == A.end");
        checkNear(ld.end.x, la.start.x, 1.0e-4, "D.end == A.start");
        checkNear(ld.end.y, la.start.y, 1.0e-4, "D.end == A.start");
        std::cout << "[PASS] Scenario 1: linked contour + equal/parallel/perpendicular/angle\n";
    }

    // Scenario 2: circles concentric + equal radius (relative).
    {
        mir::SketchDocument doc;
        auto& g = doc.geometry();
        auto& c = doc.constraints();

        const auto k1 = g.addCircle({0.0, 0.0}, 2.0);
        const auto k2 = g.addCircle({1.0, 1.0}, 3.0);
        c.add(mir::SketchConstraintType::Concentric, k1, k2);
        c.add(mir::SketchConstraintType::Equal, k1, k2);

        const auto result = mir::SketchDocumentSolver::solve(doc);
        assert(result.converged);

        const auto ck1 = getCircle(g, k1);
        const auto ck2 = getCircle(g, k2);
        checkNear(ck2.center.x, ck1.center.x, 1.0e-4, "K2 == K1 center.x");
        checkNear(ck2.center.y, ck1.center.y, 1.0e-4, "K2 == K1 center.y");
        checkNear(ck1.radius, ck2.radius, 1.0e-4, "radii equal");
        std::cout << "[PASS] Scenario 2: circles concentric + equal radius\n";
    }

    // Scenario 3: circle radius driving.
    {
        mir::SketchDocument doc;
        auto& g = doc.geometry();
        auto& c = doc.constraints();
        const auto k = g.addCircle({2.0, 2.0}, 1.5);
        c.add(mir::SketchConstraintType::Radius, k, 0, 2.5);

        const auto result = mir::SketchDocumentSolver::solve(doc);
        assert(result.converged);
        const auto ck = getCircle(g, k);
        checkNear(ck.radius, 2.5, 1.0e-4, "circle radius driving");
        std::cout << "[PASS] Scenario 3: circle radius\n";
    }

    // Scenario 4: circle tangent to line (line free to translate -> verify relation).
    {
        mir::SketchDocument doc;
        auto& g = doc.geometry();
        auto& c = doc.constraints();
        const auto l = g.addLine({0.0, 0.0}, {5.0, 0.0});
        const auto k = g.addCircle({2.0, 3.0}, 1.0);
        c.add(mir::SketchConstraintType::Tangent, l, k);

        const auto result = mir::SketchDocumentSolver::solve(doc);
        assert(result.converged);
        const auto ll = getLine(g, l);
        const auto ck = getCircle(g, k);
        const double dist = pointToLineDistance(ck.center.x, ck.center.y, ll);
        checkNear(dist, ck.radius, 1.0e-3, "tangent distance == radius");
        std::cout << "[PASS] Scenario 4: tangent circle-to-line\n";
    }

    // Scenario 5: coincident line join.
    {
        mir::SketchDocument doc;
        auto& g = doc.geometry();
        auto& c = doc.constraints();
        const auto a = g.addLine({0.0, 0.0}, {3.0, 0.0});
        const auto b = g.addLine({10.0, 10.0}, {13.0, 10.0});
        c.add(mir::SketchConstraintType::Coincident, a, b);

        const auto result = mir::SketchDocumentSolver::solve(doc);
        assert(result.converged);
        const auto la = getLine(g, a);
        const auto lb = getLine(g, b);
        checkNear(lb.start.x, la.end.x, 1.0e-4, "B.start == A.end");
        checkNear(lb.start.y, la.end.y, 1.0e-4, "B.start == A.end");
        std::cout << "[PASS] Scenario 5: coincident line join\n";
    }

    std::cout << "All SketchDocumentSolver tests passed.\n";
    return 0;
}
