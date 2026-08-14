#include "MirEngine/Sketch/SketchRectangleSolver.hpp"
#include <cassert>
#include <cmath>
int main()
{
    const auto result = mir::SketchRectangleSolver{}.solve(100.0, 60.0);
    assert(result.solver.converged);
    assert(result.solver.residual <= 1.0e-6);
    const auto& v = result.variables;
    assert(std::abs(v[0]) < 1.0e-6); assert(std::abs(v[1]) < 1.0e-6);
    assert(std::abs(v[2]-100.0) < 1.0e-6); assert(std::abs(v[3]) < 1.0e-6);
    assert(std::abs(v[4]-100.0) < 1.0e-6); assert(std::abs(v[5]-60.0) < 1.0e-6);
    assert(std::abs(v[6]) < 1.0e-6); assert(std::abs(v[7]-60.0) < 1.0e-6);
    return 0;
}
