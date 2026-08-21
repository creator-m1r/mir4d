
#pragma once

#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector3.hpp"
#include "MirEngine/Math/TransformMatrix.hpp"

#include <cstdint>
#include <string>

namespace mir
{

enum class PlaneType : std::uint8_t
{
    BaseXY = 0,
    BaseXZ,
    BaseYZ,
    UserOffset,
    UserThreePoint,
    UserAngle,
    UserParallel,
    UserPerpendicular,
    UserFace
};

constexpr std::uint32_t kBasePlaneXY = 1;
constexpr std::uint32_t kBasePlaneXZ = 2;
constexpr std::uint32_t kBasePlaneYZ = 3;

class Plane
{
public:
    Plane() = default;

    Plane(std::string name,
          PlaneType type,
          Point3 origin,
          Vector3 normal,
          Vector3 xAxis,
          std::uint32_t parentId = 0)
        : id_(0)
        , name_(std::move(name))
        , type_(type)
        , origin_(origin)
        , normal_(normal.normalized())
        , xAxis_(xAxis.normalized())
        , parentId_(parentId)
    {
    }

    [[nodiscard]] std::uint32_t id() const noexcept { return id_; }
    void setId(std::uint32_t id) noexcept { id_ = id; }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void setName(std::string name) noexcept { name_ = std::move(name); }

    [[nodiscard]] PlaneType type() const noexcept { return type_; }

    [[nodiscard]] const Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const Vector3& normal() const noexcept { return normal_; }
    [[nodiscard]] const Vector3& xAxis() const noexcept { return xAxis_; }

    [[nodiscard]] Vector3 yAxis() const noexcept
    {
        return Vector3::cross(normal_, xAxis_).normalized();
    }

    [[nodiscard]] std::uint32_t parentId() const noexcept { return parentId_; }

    [[nodiscard]] double offset() const noexcept { return offset_; }
    void setOffset(double value) noexcept { offset_ = value; }

    [[nodiscard]] double angleDeg() const noexcept { return angleDeg_; }
    void setAngleDeg(double value) noexcept { angleDeg_ = value; }

    [[nodiscard]] bool deletable() const noexcept
    {
        return type_ != PlaneType::BaseXY &&
               type_ != PlaneType::BaseXZ &&
               type_ != PlaneType::BaseYZ;
    }

    [[nodiscard]] Matrix4 localToWorld() const noexcept
    {
        const Vector3 y = yAxis();
        return Matrix4{
            xAxis_.x, y.x, normal_.x, origin_.x,
            xAxis_.y, y.y, normal_.y, origin_.y,
            xAxis_.z, y.z, normal_.z, origin_.z,
            0.0, 0.0, 0.0, 1.0};
    }

    [[nodiscard]] Matrix4 worldToLocal() const noexcept
    {
        return localToWorld().inverse();
    }

    [[nodiscard]] Point3 toWorld(double lx, double ly) const noexcept
    {
        const Vector3 world = localToWorld().transformPoint(Vector3{lx, ly, 0.0});
        return Point3{world.x, world.y, world.z};
    }

    void toLocal(const Point3& world, double& lx, double& ly) const noexcept
    {
        const Vector3 y = yAxis();
        const Vector3 rel{world.x - origin_.x, world.y - origin_.y, world.z - origin_.z};
        lx = Vector3::dot(rel, xAxis_);
        ly = Vector3::dot(rel, y);
    }

private:
    std::uint32_t id_{0};
    std::string name_;
    PlaneType type_{PlaneType::BaseXY};
    Point3 origin_{0.0, 0.0, 0.0};
    Vector3 normal_{0.0, 0.0, 1.0};
    Vector3 xAxis_{1.0, 0.0, 0.0};
    std::uint32_t parentId_{0};
    double offset_{0.0};
    double angleDeg_{0.0};
};

}
