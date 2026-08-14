#pragma once

#include "SketchVariableBinding.hpp"

#include <cstddef>
#include <vector>

namespace mir
{

class SketchSolverState
{
public:
    void resize(std::size_t count)
    {
        variables_.resize(count, 0.0);
    }

    [[nodiscard]] std::vector<double>& variables() noexcept
    {
        return variables_;
    }

    [[nodiscard]] const std::vector<double>& variables() const noexcept
    {
        return variables_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return variables_.size();
    }

    void clear() noexcept
    {
        variables_.clear();
    }

    [[nodiscard]] SketchVariableBinding& binding() noexcept
    {
        return binding_;
    }

    [[nodiscard]] const SketchVariableBinding& binding() const noexcept
    {
        return binding_;
    }

private:
    std::vector<double> variables_;
    SketchVariableBinding binding_;
};

} // namespace mir
