#pragma once

#include "SketchConstraint.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace mir
{

struct SketchEquation
{
    std::uint32_t id{0};
    SketchConstraintType constraintType{SketchConstraintType::Coincident};
    std::vector<std::uint32_t> geometryIds;
    std::function<double(const std::vector<double>&)> residual;
};

class SketchEquationSystem
{
public:
    void add(SketchEquation equation)
    {
        equations_.push_back(std::move(equation));
    }

    [[nodiscard]] const std::vector<SketchEquation>& all() const noexcept
    {
        return equations_;
    }

    void clear() noexcept
    {
        equations_.clear();
    }

private:
    std::vector<SketchEquation> equations_;
};

}
