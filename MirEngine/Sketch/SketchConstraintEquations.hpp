#pragma once

#include "SketchEquation.hpp"

#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace mir
{

struct SketchVariableMap
{

    static constexpr std::size_t line(std::uint32_t id, std::size_t stride = 4)
    {
        return static_cast<std::size_t>(id) * stride;
    }
};

class SketchConstraintEquations
{
public:
    static SketchEquation horizontal(
        std::uint32_t equationId,
        std::uint32_t geometryId)
    {
        const auto base = SketchVariableMap::line(geometryId);
        return {
            equationId,
            SketchConstraintType::Horizontal,
            {geometryId},
            [base](const std::vector<double>& v) noexcept
            {
                if (base + 3 >= v.size())
                    return 0.0;
                return v[base + 3] - v[base + 1];
            }};
    }

    static SketchEquation vertical(
        std::uint32_t equationId,
        std::uint32_t geometryId)
    {
        const auto base = SketchVariableMap::line(geometryId);
        return {
            equationId,
            SketchConstraintType::Vertical,
            {geometryId},
            [base](const std::vector<double>& v) noexcept
            {
                if (base + 2 >= v.size())
                    return 0.0;
                return v[base + 2] - v[base];
            }};
    }

    static SketchEquation distance(
        std::uint32_t equationId,
        std::uint32_t firstGeometryId,
        std::uint32_t secondGeometryId,
        double target)
    {
        const auto first = SketchVariableMap::line(firstGeometryId);
        const auto second = SketchVariableMap::line(secondGeometryId);
        return {
            equationId,
            SketchConstraintType::Distance,
            {firstGeometryId, secondGeometryId},
            [first, second, target](const std::vector<double>& v) noexcept
            {
                if (first + 1 >= v.size() || second + 1 >= v.size())
                    return 0.0;

                const double dx = v[second] - v[first];
                const double dy = v[second + 1] - v[first + 1];
                return std::sqrt(dx * dx + dy * dy) - target;
            }};
    }
};

}
