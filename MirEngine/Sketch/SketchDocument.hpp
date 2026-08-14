#pragma once

#include "SketchConstraint.hpp"
#include "SketchGeometry.hpp"

#include <cstdint>
#include <string>

namespace mir
{

class SketchDocument
{
public:
    explicit SketchDocument(std::string name = "Sketch")
        : name_(std::move(name))
    {
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return name_;
    }

    void setName(std::string name)
    {
        name_ = std::move(name);
    }

    [[nodiscard]] SketchGeometryStore& geometry() noexcept
    {
        return geometry_;
    }

    [[nodiscard]] const SketchGeometryStore& geometry() const noexcept
    {
        return geometry_;
    }

    [[nodiscard]] SketchConstraintStore& constraints() noexcept
    {
        return constraints_;
    }

    [[nodiscard]] const SketchConstraintStore& constraints() const noexcept
    {
        return constraints_;
    }

    void clear() noexcept
    {
        geometry_.clear();
        constraints_.clear();
    }

private:
    std::string name_;
    SketchGeometryStore geometry_;
    SketchConstraintStore constraints_;
};

} // namespace mir
