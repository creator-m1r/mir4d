#include "MirEngine/Geometry/Topology/Edge.hpp"

#include <cassert>

int main()
{
    mir::Edge edge;
    assert(edge.loops().empty());

    edge.addLoop(mir::LoopID{});
    assert(edge.loops().size() == 1);

    edge.addLoop(mir::LoopID{});
    assert(edge.loops().size() == 2);

    return 0;
}
