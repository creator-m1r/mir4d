#include "MirEngine/Sketch/SketchRectangleProfileBuilder.hpp"
#include <cassert>
#include <cmath>
#include <vector>
int main()
{
    const std::vector<double> solved{0.0,0.0,100.0,0.0,100.0,60.0,0.0,60.0};
    const auto profile = mir::SketchRectangleProfileBuilder::build(solved);
    assert(profile.closed && profile.valid && !profile.selfIntersecting);
    assert(profile.geometryIDs.size() == 4);
    assert(std::abs(profile.signedArea - 6000.0) < 1.0e-8);
    assert(profile.isUsableForSolidFeature());
    return 0;
}
