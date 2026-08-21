#pragma once

#include "SketchConstraint.hpp"
#include "SketchGeometry.hpp"
#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/TransformMatrix.hpp"

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

    [[nodiscard]] std::uint32_t planeId() const noexcept { return planeId_; }
    void setPlane(std::uint32_t id, const Matrix4& localTransform) noexcept
    {
        planeId_ = id;
        localTransform_ = localTransform;
    }

    [[nodiscard]] Point3 toWorld(double lx, double ly) const noexcept
    {
        const Vector3 world = localTransform_.transformPoint(Vector3{lx, ly, 0.0});
        return Point3{world.x, world.y, world.z};
    }

    void toLocal(const Point3& world, double& lx, double& ly) const noexcept
    {
        const Vector3 xAxis{localTransform_(0, 0), localTransform_(1, 0), localTransform_(2, 0)};
        const Vector3 yAxis{localTransform_(0, 1), localTransform_(1, 1), localTransform_(2, 1)};
        const Vector3 origin{localTransform_(0, 3), localTransform_(1, 3), localTransform_(2, 3)};
        const Vector3 rel{world.x - origin.x, world.y - origin.y, world.z - origin.z};
        lx = Vector3::dot(rel, xAxis);
        ly = Vector3::dot(rel, yAxis);
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
    std::uint32_t planeId_{0};
    Matrix4 localTransform_{Matrix4::identity()};
    SketchGeometryStore geometry_;
    SketchConstraintStore constraints_;
};

}
