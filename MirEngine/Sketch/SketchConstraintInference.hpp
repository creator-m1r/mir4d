#pragma once

#include "SketchInferenceEngine.hpp"
#include "SketchDragResolver.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

class SketchConstraintInference
{
public:
    explicit SketchConstraintInference(double minimumConfidence = 0.85)
        : minimumConfidence_(minimumConfidence)
    {
    }

    [[nodiscard]] std::vector<SketchInference> accepted(
        const SketchDragResolution& resolution) const
    {
        std::vector<SketchInference> result;
        for (const auto& inference : resolution.inferences)
        {
            if (inference.confidence >= minimumConfidence_)
                result.push_back(inference);
        }
        return result;
    }

private:
    double minimumConfidence_;
};

}
