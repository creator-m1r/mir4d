// MirEngine/Planes/Plane.hpp
// =================================================================================
// Рабочая плоскость (Work Plane) — базовая сущность подсистемы эскизов МИР 4D.
//
// Плоскость имеет локальную систему координат (origin, X, Y, normal) и
// трансформируется в мировую систему координат. Эскиз хранит геометрию в
// локальной системе плоскости (Z = 0 всегда лежит на плоскости).
//
// Сущность намеренно лёгкая и header-only, как остальная подсистема Sketch,
// чтобы не требовать изменений CMake-целей STATIC-библиотек.
// =================================================================================

#pragma once

#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector3.hpp"
#include "MirEngine/Math/TransformMatrix.hpp"

#include <cstdint>
#include <string>

namespace mir
{

/// Тип рабочей плоскости (ТЗ раздел 3).
enum class PlaneType : std::uint8_t
{
    BaseXY = 0,          ///< системная XY
    BaseXZ,              ///< системная XZ
    BaseYZ,              ///< системная YZ
    UserOffset,          ///< параллельно родителю со смещением
    UserThreePoint,      ///< по трём точкам
    UserAngle,           ///< повёрнута на угол относительно родителя
    UserParallel,        ///< параллельно выбранной плоскости
    UserPerpendicular,   ///< перпендикулярно выбранной плоскости
    UserFace             ///< по грани тела (позже)
};

/// Базовые идентификаторы системных плоскостей (фиксированы в рамках проекта).
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

    // ── Identity ──────────────────────────────────────────────────────────
    [[nodiscard]] std::uint32_t id() const noexcept { return id_; }
    void setId(std::uint32_t id) noexcept { id_ = id; }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void setName(std::string name) noexcept { name_ = std::move(name); }

    [[nodiscard]] PlaneType type() const noexcept { return type_; }

    // ── Geometry ──────────────────────────────────────────────────────────
    [[nodiscard]] const Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const Vector3& normal() const noexcept { return normal_; }
    [[nodiscard]] const Vector3& xAxis() const noexcept { return xAxis_; }

    /// Локальная ось Y вычисляется как normal × xAxis (правая тройка).
    [[nodiscard]] Vector3 yAxis() const noexcept
    {
        return Vector3::cross(normal_, xAxis_).normalized();
    }

    [[nodiscard]] std::uint32_t parentId() const noexcept { return parentId_; }

    // ── Parameters (ТЗ раздел 3.3 / 3.4) ──────────────────────────────────
    [[nodiscard]] double offset() const noexcept { return offset_; }
    void setOffset(double value) noexcept { offset_ = value; }

    [[nodiscard]] double angleDeg() const noexcept { return angleDeg_; }
    void setAngleDeg(double value) noexcept { angleDeg_ = value; }

    /// Системные плоскости удалять нельзя (ТЗ раздел 3.1).
    [[nodiscard]] bool deletable() const noexcept
    {
        return type_ != PlaneType::BaseXY &&
               type_ != PlaneType::BaseXZ &&
               type_ != PlaneType::BaseYZ;
    }

    // ── Coordinate transforms (ТЗ раздел 6) ───────────────────────────────
    /// Локальная СК → Мировая: basis = (xAxis, yAxis, normal), сдвиг = origin.
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

    /// Точка эскиза (lx, ly, 0) → мировая точка.
    [[nodiscard]] Point3 toWorld(double lx, double ly) const noexcept
    {
        const Vector3 world = localToWorld().transformPoint(Vector3{lx, ly, 0.0});
        return Point3{world.x, world.y, world.z};
    }

    /// Мировая точка → локальные (lx, ly); z игнорируется (проекция на плоскость).
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

} // namespace mir
