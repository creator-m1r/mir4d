#pragma once

#include "../Core/Identity/ObjectId.hpp"

#include <vector>

namespace mir::io
{

struct ExportOptions
{
    bool binaryStl{true};
    bool selectionOnly{false};
    double unitScale{1.0};
    double linearDeflection{0.1};

    // Empty means the whole scene when selectionOnly is false.
    std::vector<mir4d::ObjectId> selection;
};

} // namespace mir::io
