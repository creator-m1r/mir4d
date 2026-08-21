// MirEngine/Planes/PlaneFactory.hpp
// =================================================================================
// Фабрика рабочих плоскостей (ТЗ раздел 3).
//
// Создаёт базовые координатные плоскости и пользовательские плоскости:
//   - по смещению (параллельно родителю);
//   - по трём точкам;
//   - под углом к родителю вокруг заданной оси.
//
// Фабрика не владеет плоскостями — она возвращает std::shared_ptr<Plane>,
// владение передаётся PlaneStore / документу.
// =================================================================================

#pragma once

#include "Plane.hpp"

#include <cmath>
#include <memory>

namespace mir
{

class PlaneFactory
{
public:
    PlaneFactory() = delete;

    /// Системные координатные плоскости. Возвращают готовые объекты с
    /// фиксированными идентификаторами (kBasePlaneXY/XZ/YZ).
    static std::shared_ptr<Plane> createBaseXY()
    {
        auto p = std::make_shared<Plane>("XY Plane", PlaneType::BaseXY,
                                         Point3{0.0, 0.0, 0.0},
                                         Vector3{0.0, 0.0, 1.0},
                                         Vector3{1.0, 0.0, 0.0});
        p->setId(kBasePlaneXY);
        return p;
    }

    static std::shared_ptr<Plane> createBaseXZ()
    {
        auto p = std::make_shared<Plane>("XZ Plane", PlaneType::BaseXZ,
                                         Point3{0.0, 0.0, 0.0},
                                         Vector3{0.0, 1.0, 0.0},
                                         Vector3{1.0, 0.0, 0.0});
        p->setId(kBasePlaneXZ);
        return p;
    }

    static std::shared_ptr<Plane> createBaseYZ()
    {
        auto p = std::make_shared<Plane>("YZ Plane", PlaneType::BaseYZ,
                                         Point3{0.0, 0.0, 0.0},
                                         Vector3{1.0, 0.0, 0.0},
                                         Vector3{0.0, 1.0, 0.0});
        p->setId(kBasePlaneYZ);
        return p;
    }

    /// Плоскость со смещением: параллельна родителю, сдвинута на offset
    /// вдоль нормали родителя (ТЗ раздел 3.3).
    static std::shared_ptr<Plane> createOffset(const Plane& parent,
                                               double offset,
                                               std::uint32_t id = 0,
                                               const std::string& name = "User Plane")
    {
        const Point3 origin = Point3{
            parent.origin().x + parent.normal().x * offset,
            parent.origin().y + parent.normal().y * offset,
            parent.origin().z + parent.normal().z * offset};
        auto p = std::make_shared<Plane>(name, PlaneType::UserOffset,
                                         origin, parent.normal(), parent.xAxis(),
                                         parent.id());
        p->setOffset(offset);
        if (id != 0) p->setId(id);
        return p;
    }

    /// Плоскость по трём точкам (ТЗ раздел 3.2).
    static std::shared_ptr<Plane> createThreePoint(const Point3& p1,
                                                   const Point3& p2,
                                                   const Point3& p3,
                                                   std::uint32_t id = 0,
                                                   const std::string& name = "User Plane")
    {
        const Vector3 v12{p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
        const Vector3 v13{p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
        Vector3 normal = Vector3::cross(v12, v13);
        if (normal.lengthSquared() < 1e-18)
            normal = Vector3{0.0, 0.0, 1.0};
        Vector3 xAxis = v12;
        if (xAxis.lengthSquared() < 1e-18)
            xAxis = Vector3{1.0, 0.0, 0.0};
        auto p = std::make_shared<Plane>(name, PlaneType::UserThreePoint,
                                         p1, normal, xAxis);
        if (id != 0) p->setId(id);
        return p;
    }

    /// Плоскость под углом: базис родителя повёрнут вокруг оси axis
    /// на angleDeg (ТЗ раздел 3.4). Ось нормализуется.
    static std::shared_ptr<Plane> createAngle(const Plane& parent,
                                              const Vector3& axis,
                                              double angleDeg,
                                              std::uint32_t id = 0,
                                              const std::string& name = "User Plane")
    {
        Vector3 a = axis.normalized();
        const double rad = angleDeg * 3.14159265358979323846 / 180.0;
        const double c = std::cos(rad);
        const double s = std::sin(rad);
        const double k = 1.0 - c;

        auto rotate = [&](const Vector3& v) -> Vector3
        {
            return Vector3{
                v.x * (c + a.x * a.x * k) + v.y * (a.x * a.y * k - a.z * s) + v.z * (a.x * a.z * k + a.y * s),
                v.x * (a.y * a.x * k + a.z * s) + v.y * (c + a.y * a.y * k) + v.z * (a.y * a.z * k - a.x * s),
                v.x * (a.z * a.x * k - a.y * s) + v.y * (a.z * a.y * k + a.x * s) + v.z * (c + a.z * a.z * k)};
        };

        const Vector3 normal = rotate(parent.normal()).normalized();
        const Vector3 xAxis = rotate(parent.xAxis()).normalized();
        auto p = std::make_shared<Plane>(name, PlaneType::UserAngle,
                                         parent.origin(), normal, xAxis, parent.id());
        p->setAngleDeg(angleDeg);
        if (id != 0) p->setId(id);
        return p;
    }
};

} // namespace mir
