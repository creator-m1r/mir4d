#pragma once

#include "OpticalMaterialField.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace mir
{

struct OpticalGridDimensions
{
    std::size_t x{1};
    std::size_t y{1};
    std::size_t z{1};
};

struct OpticalVolumeBounds
{
    WorldPosition minimum{};
    WorldPosition maximum{};
};

class OpticalVolumeGrid
{
public:
    OpticalVolumeGrid(
        OpticalGridDimensions dimensions = {},
        OpticalVolumeBounds bounds = {})
        : dimensions_(sanitise(dimensions)),
          bounds_(bounds),
          fields_(dimensions_.x * dimensions_.y * dimensions_.z)
    {
    }

    [[nodiscard]] OpticalGridDimensions dimensions() const noexcept
    {
        return dimensions_;
    }

    [[nodiscard]] const OpticalVolumeBounds& bounds() const noexcept
    {
        return bounds_;
    }

    void setBounds(const OpticalVolumeBounds& bounds) noexcept
    {
        bounds_ = bounds;
    }

    [[nodiscard]] OpticalMaterialField& field(
        std::size_t x,
        std::size_t y,
        std::size_t z) noexcept
    {
        return fields_[index(x, y, z)];
    }

    [[nodiscard]] const OpticalMaterialField& field(
        std::size_t x,
        std::size_t y,
        std::size_t z) const noexcept
    {
        return fields_[index(x, y, z)];
    }

    [[nodiscard]] float sample(
        WorldPosition position,
        float wavelengthNm,
        const SpectralLight& light) const noexcept
    {
        const auto coordinate = coordinateAt(position);
        return field(coordinate.x, coordinate.y, coordinate.z)
            .response(light, wavelengthNm);
    }

private:
    struct GridCoordinate
    {
        std::size_t x{0};
        std::size_t y{0};
        std::size_t z{0};
    };

    [[nodiscard]] static OpticalGridDimensions sanitise(
        OpticalGridDimensions dimensions) noexcept
    {
        dimensions.x = std::max<std::size_t>(1, dimensions.x);
        dimensions.y = std::max<std::size_t>(1, dimensions.y);
        dimensions.z = std::max<std::size_t>(1, dimensions.z);
        return dimensions;
    }

    [[nodiscard]] std::size_t index(
        std::size_t x,
        std::size_t y,
        std::size_t z) const noexcept
    {
        x = std::min(x, dimensions_.x - 1);
        y = std::min(y, dimensions_.y - 1);
        z = std::min(z, dimensions_.z - 1);
        return x + dimensions_.x * (y + dimensions_.y * z);
    }

    [[nodiscard]] GridCoordinate coordinateAt(WorldPosition position) const noexcept
    {
        const double width = bounds_.maximum.x - bounds_.minimum.x;
        const double height = bounds_.maximum.y - bounds_.minimum.y;
        const double depth = bounds_.maximum.z - bounds_.minimum.z;

        const double nx = width > 0.0
            ? (position.x - bounds_.minimum.x) / width
            : 0.0;
        const double ny = height > 0.0
            ? (position.y - bounds_.minimum.y) / height
            : 0.0;
        const double nz = depth > 0.0
            ? (position.z - bounds_.minimum.z) / depth
            : 0.0;

        return {
            toIndex(nx, dimensions_.x),
            toIndex(ny, dimensions_.y),
            toIndex(nz, dimensions_.z)};
    }

    [[nodiscard]] static std::size_t toIndex(
        double normalised,
        std::size_t dimension) noexcept
    {
        if (dimension <= 1)
            return 0;

        const double value = std::clamp(normalised, 0.0, 1.0);
        const double scaled = value * static_cast<double>(dimension - 1);
        return std::min(
            dimension - 1,
            static_cast<std::size_t>(scaled));
    }

    OpticalGridDimensions dimensions_{};
    OpticalVolumeBounds bounds_{};
    std::vector<OpticalMaterialField> fields_;
};

} // namespace mir
