#pragma once

#include "SketchCommandHistory.hpp"

#include <cstdint>

namespace mir
{

class AddConstraintCommand final : public ISketchCommand
{
public:
    AddConstraintCommand(
        SketchConstraintType type,
        std::uint32_t firstGeometry,
        std::uint32_t secondGeometry = 0,
        double value = 0.0,
        bool driving = true)
        : type_(type), firstGeometry_(firstGeometry), secondGeometry_(secondGeometry), value_(value), driving_(driving)
    {
    }

    bool execute(SketchDocument& document) override
    {
        if (constraintId_ != 0)
            return true;

        constraintId_ = document.constraints().add(
            type_, firstGeometry_, secondGeometry_, value_, driving_);
        return constraintId_ != 0;
    }

    bool undo(SketchDocument& document) override
    {
        if (constraintId_ == 0)
            return false;

        const bool removed = document.constraints().remove(constraintId_);
        if (removed)
            constraintId_ = 0;
        return removed;
    }

    [[nodiscard]] std::uint32_t constraintId() const noexcept
    {
        return constraintId_;
    }

private:
    SketchConstraintType type_;
    std::uint32_t firstGeometry_{0};
    std::uint32_t secondGeometry_{0};
    double value_{0.0};
    bool driving_{true};
    std::uint32_t constraintId_{0};
};

} // namespace mir
