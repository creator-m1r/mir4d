#pragma once

namespace mir::io
{

struct ImportOptions
{
    bool triangulate{true};
    bool generateNormals{true};
    bool joinIdenticalVertices{true};
    double linearDeflection{0.1};
    double angularDeflection{0.5};
    double unitScale{1.0};
};

} // namespace mir::io
