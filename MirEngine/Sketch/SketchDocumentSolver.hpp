// MirEngine/Sketch/SketchDocumentSolver.hpp
//
// Universal sketch constraint solver for MIR 4D.

#pragma once

#include "MirEngine/Sketch/SketchDocument.hpp"
#include "MirEngine/Sketch/SketchSolverNewton.hpp"

#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir
{

struct SketchSolveBinding
{
    std::unordered_map<std::uint32_t, std::size_t> base;
    std::size_t count{0};

    std::size_t offset(std::uint32_t id) const { return base.at(id); }
};

enum class SketchPointRole : std::uint8_t
{
    Start = 0,
    End = 1,
    Center = 2
};

inline std::pair<double, double> readPoint(
    const std::vector<double>& x,
    const SketchSolveBinding& binding,
    std::uint32_t id,
    SketchPointRole role)
{
    const std::size_t o = binding.base.at(id);
    switch (role)
    {
        case SketchPointRole::Start:
            return {x[o + 0], x[o + 1]};
        case SketchPointRole::End:
            return {x[o + 2], x[o + 3]};
        case SketchPointRole::Center:
            return {x[o + 0], x[o + 1]};
    }
    return {x[o + 0], x[o + 1]};
}

inline double readRadius(const std::vector<double>& x, const SketchSolveBinding& binding, std::uint32_t id)
{
    return x[binding.base.at(id) + 2];
}

struct SketchDocumentSolverOptions
{
    double tolerance{1.0e-7};
    std::uint32_t maxIterations{200};
    double finiteDifferenceStep{1.0e-6};
    double maxStep{1.0e6};
};

class SketchDocumentSolver
{
public:
    using Options = SketchDocumentSolverOptions;

    static SketchNewtonResult solve(SketchDocument& doc, const Options& options = Options{})
    {
        auto& store = doc.geometry();
        const auto& constraints = doc.constraints();

        SketchSolveBinding binding = buildBinding(store);
        std::vector<double> variables = initialVariables(store, binding);

        SketchEquationSystem system;
        for (const auto& con : constraints.all())
        {
            auto eqs = buildConstraintEquations(con, store, binding);
            for (auto& eq : eqs)
            {
                system.add(std::move(eq));
            }
        }

        if (system.all().empty())
        {
            return SketchNewtonResult{true, 0, 0.0};
        }

        SketchNewtonOptions nopt;
        nopt.tolerance = options.tolerance;
        nopt.maxIterations = options.maxIterations;
        nopt.finiteDifferenceStep = options.finiteDifferenceStep;
        nopt.maxStep = options.maxStep;

        SketchSolverNewton solver;
        const auto result = solver.solve(system, variables, nopt);

        if (result.converged)
        {
            writeBack(store, variables, binding);
        }
        return result;
    }

private:
    static SketchSolveBinding buildBinding(const SketchGeometryStore& store)
    {
        SketchSolveBinding binding;
        for (const auto& geo : store.all())
        {
            if (const auto* l = std::get_if<SketchLine2D>(&geo))
            {
                binding.base[l->id] = binding.count;
                binding.count += 4;
            }
            else if (const auto* c = std::get_if<SketchCircle2D>(&geo))
            {
                binding.base[c->id] = binding.count;
                binding.count += 3;
            }
            else if (const auto* a = std::get_if<SketchArc2D>(&geo))
            {
                binding.base[a->id] = binding.count;
                binding.count += 5;
            }
        }
        return binding;
    }

    static std::vector<double> initialVariables(
        const SketchGeometryStore& store,
        const SketchSolveBinding& binding)
    {
        std::vector<double> x(binding.count, 0.0);
        for (const auto& geo : store.all())
        {
            if (const auto* l = std::get_if<SketchLine2D>(&geo))
            {
                const std::size_t o = binding.base.at(l->id);
                x[o + 0] = l->start.x;
                x[o + 1] = l->start.y;
                x[o + 2] = l->end.x;
                x[o + 3] = l->end.y;
            }
            else if (const auto* c = std::get_if<SketchCircle2D>(&geo))
            {
                const std::size_t o = binding.base.at(c->id);
                x[o + 0] = c->center.x;
                x[o + 1] = c->center.y;
                x[o + 2] = c->radius;
            }
            else if (const auto* a = std::get_if<SketchArc2D>(&geo))
            {
                const std::size_t o = binding.base.at(a->id);
                x[o + 0] = a->center.x;
                x[o + 1] = a->center.y;
                x[o + 2] = a->radius;
                x[o + 3] = a->startAngle;
                x[o + 4] = a->endAngle;
            }
        }
        return x;
    }

    static void writeBack(
        SketchGeometryStore& store,
        const std::vector<double>& x,
        const SketchSolveBinding& binding)
    {
        for (auto& geo : store.mutableAllForSolver())
        {
            if (auto* l = std::get_if<SketchLine2D>(&geo))
            {
                const std::size_t o = binding.base.at(l->id);
                l->start.x = x[o + 0];
                l->start.y = x[o + 1];
                l->end.x = x[o + 2];
                l->end.y = x[o + 3];
            }
            else if (auto* c = std::get_if<SketchCircle2D>(&geo))
            {
                const std::size_t o = binding.base.at(c->id);
                c->center.x = x[o + 0];
                c->center.y = x[o + 1];
                c->radius = x[o + 2];
            }
            else if (auto* a = std::get_if<SketchArc2D>(&geo))
            {
                const std::size_t o = binding.base.at(a->id);
                a->center.x = x[o + 0];
                a->center.y = x[o + 1];
                a->radius = x[o + 2];
                a->startAngle = x[o + 3];
                a->endAngle = x[o + 4];
            }
        }
    }

    static std::vector<SketchEquation> buildConstraintEquations(
        const SketchConstraint& con,
        const SketchGeometryStore& store,
        const SketchSolveBinding& binding)
    {
        std::vector<SketchEquation> out;
        std::uint32_t eid = con.id * 16;

        // Defensive: never let a constraint referencing a geometry that is not
        // part of the solver binding (e.g. a spline, or a stale/removed id)
        // throw std::out_of_range across the C ABI. Skip such equations.
        if (binding.base.find(con.firstGeometry) == binding.base.end())
            return out;
        if (con.secondGeometry != 0 &&
            binding.base.find(con.secondGeometry) == binding.base.end())
            return out;

        auto push = [&](std::uint32_t localId,
                        std::function<double(const std::vector<double>&)> f)
        {
            SketchEquation eq;
            eq.id = eid + localId;
            eq.constraintType = con.type;
            eq.geometryIds = {con.firstGeometry, con.secondGeometry};
            eq.residual = std::move(f);
            out.push_back(std::move(eq));
        };

        switch (con.type)
        {
            case SketchConstraintType::Horizontal:
            {
                const auto* g = store.find(con.firstGeometry);
                if (!g || !std::holds_alternative<SketchLine2D>(*g))
                    return out;
                const std::size_t o = binding.base.at(con.firstGeometry);
                push(0, [o](const std::vector<double>& x) { return x[o + 1] - x[o + 3]; });
                return out;
            }
            case SketchConstraintType::Vertical:
            {
                const auto* g = store.find(con.firstGeometry);
                if (!g || !std::holds_alternative<SketchLine2D>(*g))
                    return out;
                const std::size_t o = binding.base.at(con.firstGeometry);
                push(0, [o](const std::vector<double>& x) { return x[o + 0] - x[o + 2]; });
                return out;
            }
            case SketchConstraintType::Distance:
            {
                const auto* g1 = store.find(con.firstGeometry);
                if (!g1)
                    return out;
                const double target = con.value;
                if (con.secondGeometry == 0)
                {
                    const std::size_t o = binding.base.at(con.firstGeometry);
                    if (std::holds_alternative<SketchLine2D>(*g1))
                    {
                        push(0, [o, target](const std::vector<double>& x)
                             {
                                 const double dx = x[o + 2] - x[o + 0];
                                 const double dy = x[o + 3] - x[o + 1];
                                 return std::hypot(dx, dy) - target;
                             });
                    }
                    else if (std::holds_alternative<SketchCircle2D>(*g1))
                    {
                        push(0, [o, target](const std::vector<double>& x) { return 2.0 * x[o + 2] - target; });
                    }
                    return out;
                }
                const auto* g2 = store.find(con.secondGeometry);
                if (!g2)
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                push(0, [o1, o2, target](const std::vector<double>& x)
                     {
                         const double dx = x[o2 + 0] - x[o1 + 0];
                         const double dy = x[o2 + 1] - x[o1 + 1];
                         return std::hypot(dx, dy) - target;
                     });
                return out;
            }
            case SketchConstraintType::Equal:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2)
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                if (std::holds_alternative<SketchLine2D>(*g1) &&
                    std::holds_alternative<SketchLine2D>(*g2))
                {
                    push(0, [o1, o2](const std::vector<double>& x)
                     {
                         const double l1 = std::hypot(x[o1 + 2] - x[o1 + 0], x[o1 + 3] - x[o1 + 1]);
                         const double l2 = std::hypot(x[o2 + 2] - x[o2 + 0], x[o2 + 3] - x[o2 + 1]);
                         return l1 - l2;
                     });
                }
                else if (std::holds_alternative<SketchCircle2D>(*g1) &&
                         std::holds_alternative<SketchCircle2D>(*g2))
                {
                    push(0, [o1, o2](const std::vector<double>& x) { return x[o1 + 2] - x[o2 + 2]; });
                }
                return out;
            }
            case SketchConstraintType::Parallel:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2 || !std::holds_alternative<SketchLine2D>(*g1) ||
                    !std::holds_alternative<SketchLine2D>(*g2))
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                push(0, [o1, o2](const std::vector<double>& x)
                 {
                     const double ax = x[o1 + 2] - x[o1 + 0];
                     const double ay = x[o1 + 3] - x[o1 + 1];
                     const double bx = x[o2 + 2] - x[o2 + 0];
                     const double by = x[o2 + 3] - x[o2 + 1];
                     return ax * by - ay * bx;
                 });
                return out;
            }
            case SketchConstraintType::Perpendicular:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2 || !std::holds_alternative<SketchLine2D>(*g1) ||
                    !std::holds_alternative<SketchLine2D>(*g2))
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                push(0, [o1, o2](const std::vector<double>& x)
                 {
                     const double ax = x[o1 + 2] - x[o1 + 0];
                     const double ay = x[o1 + 3] - x[o1 + 1];
                     const double bx = x[o2 + 2] - x[o2 + 0];
                     const double by = x[o2 + 3] - x[o2 + 1];
                     return ax * bx + ay * by;
                 });
                return out;
            }
            case SketchConstraintType::Tangent:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2)
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                if (std::holds_alternative<SketchCircle2D>(*g1) &&
                    std::holds_alternative<SketchCircle2D>(*g2))
                {
                    push(0, [o1, o2](const std::vector<double>& x)
                     {
                         const double dx = x[o2 + 0] - x[o1 + 0];
                         const double dy = x[o2 + 1] - x[o1 + 1];
                         const double dc = std::hypot(dx, dy);
                         return dc - (x[o1 + 2] + x[o2 + 2]);
                     });
                    return out;
                }
                const bool lineFirst = std::holds_alternative<SketchLine2D>(*g1);
                const std::size_t lo = lineFirst ? o1 : o2;
                const std::size_t co = lineFirst ? o2 : o1;
                push(0, [lo, co](const std::vector<double>& x)
                 {
                     const double px = x[lo + 0];
                     const double py = x[lo + 1];
                     const double qx = x[lo + 2];
                     const double qy = x[lo + 3];
                     const double cx = x[co + 0];
                     const double cy = x[co + 1];
                     const double dx = qx - px;
                     const double dy = qy - py;
                     const double len = std::hypot(dx, dy);
                     if (len < 1.0e-9)
                         return 0.0;
                     const double dist = std::fabs((cx - px) * dy - (cy - py) * dx) / len;
                     return dist - x[co + 2];
                 });
                return out;
            }
            case SketchConstraintType::Concentric:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2 || !std::holds_alternative<SketchCircle2D>(*g1) ||
                    !std::holds_alternative<SketchCircle2D>(*g2))
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                push(0, [o1, o2](const std::vector<double>& x) { return x[o1 + 0] - x[o2 + 0]; });
                push(1, [o1, o2](const std::vector<double>& x) { return x[o1 + 1] - x[o2 + 1]; });
                return out;
            }
            case SketchConstraintType::Radius:
            {
                const auto* g1 = store.find(con.firstGeometry);
                if (!g1 || !std::holds_alternative<SketchCircle2D>(*g1))
                    return out;
                const std::size_t o = binding.base.at(con.firstGeometry);
                const double target = con.value;
                push(0, [o, target](const std::vector<double>& x) { return x[o + 2] - target; });
                return out;
            }
            case SketchConstraintType::Diameter:
            {
                const auto* g1 = store.find(con.firstGeometry);
                if (!g1 || !std::holds_alternative<SketchCircle2D>(*g1))
                    return out;
                const std::size_t o = binding.base.at(con.firstGeometry);
                const double target = con.value;
                push(0, [o, target](const std::vector<double>& x) { return 2.0 * x[o + 2] - target; });
                return out;
            }
            case SketchConstraintType::Angle:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2 || !std::holds_alternative<SketchLine2D>(*g1) ||
                    !std::holds_alternative<SketchLine2D>(*g2))
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                const double target = con.value;
                push(0, [o1, o2, target](const std::vector<double>& x)
                 {
                     const double ax = x[o1 + 2] - x[o1 + 0];
                     const double ay = x[o1 + 3] - x[o1 + 1];
                     const double bx = x[o2 + 2] - x[o2 + 0];
                     const double by = x[o2 + 3] - x[o2 + 1];
                     const double m1 = std::hypot(ax, ay);
                     const double m2 = std::hypot(bx, by);
                     if (m1 < 1.0e-9 || m2 < 1.0e-9)
                         return 0.0;
                     double ang = std::acos(std::clamp((ax * bx + ay * by) / (m1 * m2), -1.0, 1.0));
                     return ang - target;
                 });
                return out;
            }
            case SketchConstraintType::Coincident:
            {
                const auto* g1 = store.find(con.firstGeometry);
                const auto* g2 = store.find(con.secondGeometry);
                if (!g1 || !g2 || !std::holds_alternative<SketchLine2D>(*g1) ||
                    !std::holds_alternative<SketchLine2D>(*g2))
                    return out;
                const std::size_t o1 = binding.base.at(con.firstGeometry);
                const std::size_t o2 = binding.base.at(con.secondGeometry);
                push(0, [o1, o2](const std::vector<double>& x) { return x[o1 + 2] - x[o2 + 0]; });
                push(1, [o1, o2](const std::vector<double>& x) { return x[o1 + 3] - x[o2 + 1]; });
                return out;
            }
            case SketchConstraintType::Symmetric:
                return out;
            default:
                return out;
        }
    }
};

} // namespace mir
