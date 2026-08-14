// MirEngine/Geometry/Solid/Pyramid.hpp
// 🔺 Пирамида — твёрдое тело с прямоугольным основанием и вершиной.
//
// Пирамида задаётся центром основания, векторами, определяющими
// ширину и глубину (оси основания), и направлением вверх к вершине.
// Вершина находится на расстоянии height от центра основания вдоль
// направления apexDirection.
//
// В отличие от конуса, основание пирамиды — прямоугольник, а не круг.
// Это делает пирамиду удобным примитивом для построения призматических
// форм, клиньев, крыш и других элементов архитектуры и машиностроения.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Solid.hpp"
#include "../Point/Point3.hpp"
#include "../Direction/Direction3.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../../Math/Bounds/AABB.hpp"
#include "../../Math/Transform.hpp"
#include "../Ray/Ray3.hpp"
#include "../Plane/Plane.hpp"
#include <cmath>
#include <array>
#include <memory>

namespace mir {

class Pyramid : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    Pyramid(const Point3& baseCenter,
            const Direction3& baseDirX, Scalar baseWidth,
            const Direction3& baseDirY, Scalar baseDepth,
            const Direction3& apexDirection, Scalar height) noexcept
        : m_baseCenter(baseCenter)
        , m_baseDirX(baseDirX)
        , m_baseWidth(std::abs(baseWidth))
        , m_baseDirY(baseDirY)
        , m_baseDepth(std::abs(baseDepth))
        , m_apexDirection(apexDirection)
        , m_height(std::abs(height))
    {}

    Pyramid(const Point3& baseCenter, Scalar baseWidth, Scalar baseDepth, Scalar height) noexcept
        : Pyramid(baseCenter, Direction3::unitX(), baseWidth,
                  Direction3::unitY(), baseDepth,
                  Direction3::unitZ(), height)
    {}

    // ── Доступ к параметрам ─────────────────────────────────
    [[nodiscard]] Point3 baseCenter()     const noexcept { return m_baseCenter; }
    [[nodiscard]] Direction3 baseDirX()   const noexcept { return m_baseDirX; }
    [[nodiscard]] Direction3 baseDirY()   const noexcept { return m_baseDirY; }
    [[nodiscard]] Scalar baseWidth()      const noexcept { return m_baseWidth; }
    [[nodiscard]] Scalar baseDepth()      const noexcept { return m_baseDepth; }
    [[nodiscard]] Direction3 apexDir()    const noexcept { return m_apexDirection; }
    [[nodiscard]] Scalar height()        const noexcept { return m_height; }

    [[nodiscard]] Point3 apex() const noexcept {
        return m_baseCenter + m_apexDirection.asVector() * m_height;
    }

    // ── Реализация Solid ────────────────────────────────────

    [[nodiscard]] Scalar volume() const override {
        return (Scalar(1) / Scalar(3)) * m_baseWidth * m_baseDepth * m_height;
    }

    [[nodiscard]] Scalar surfaceArea() const override {
        Scalar halfW = m_baseWidth * Scalar(0.5);
        Scalar halfD = m_baseDepth * Scalar(0.5);
        Point3 apexPt = apex();

        Point3 corners[4] = {
            m_baseCenter - m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD,
            m_baseCenter - m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD
        };

        Scalar sideArea = Scalar(0);
        for (int i = 0; i < 4; ++i) {
            Point3 a = corners[i];
            Point3 b = corners[(i + 1) % 4];
            Vector3 ab = b - a;
            Vector3 ao = apexPt - a;
            sideArea += Vector3::cross(ab, ao).length() * Scalar(0.5);
        }

        return m_baseWidth * m_baseDepth + sideArea;
    }

    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar alongApex = Vector3::dot(toPoint, m_apexDirection.asVector());
        if (alongApex < -tolerance || alongApex > m_height + tolerance) return false;

        Scalar scale = Scalar(1) - alongApex / m_height;
        Scalar currentHalfW = m_baseWidth * Scalar(0.5) * scale;
        Scalar currentHalfD = m_baseDepth * Scalar(0.5) * scale;

        Vector3 local = toPoint - m_apexDirection.asVector() * alongApex;
        Scalar localX = Vector3::dot(local, m_baseDirX.asVector());
        Scalar localY = Vector3::dot(local, m_baseDirY.asVector());

        return std::abs(localX) <= currentHalfW + tolerance &&
               std::abs(localY) <= currentHalfD + tolerance;
    }

    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        Point3 apexPt = apex();
        Scalar halfW = m_baseWidth * Scalar(0.5);
        Scalar halfD = m_baseDepth * Scalar(0.5);

        Point3 corners[4] = {
            m_baseCenter - m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD,
            m_baseCenter - m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD
        };

        Point3 bestPoint = corners[0];
        Scalar bestDist = Point3::distance(point, bestPoint);

        Plane basePlane(m_baseCenter, m_apexDirection.opposite());
        Point3 projBase = basePlane.project(point);
        if (contains(projBase, Scalar(1e-10))) {
            Scalar d = Point3::distance(point, projBase);
            if (d < bestDist) { bestDist = d; bestPoint = projBase; }
        }

        for (int i = 0; i < 4; ++i) {
            Point3 a = corners[i];
            Point3 b = corners[(i + 1) % 4];
            Plane facePlane(a, Direction3::fromVector(Vector3::cross(b - a, apexPt - a)));
            Point3 proj = facePlane.project(point);

            Vector3 v0 = b - a;
            Vector3 v1 = apexPt - a;
            Vector3 v2 = proj - a;
            Scalar dot00 = Vector3::dot(v0, v0);
            Scalar dot01 = Vector3::dot(v0, v1);
            Scalar dot02 = Vector3::dot(v0, v2);
            Scalar dot11 = Vector3::dot(v1, v1);
            Scalar dot12 = Vector3::dot(v1, v2);
            Scalar invDenom = Scalar(1) / (dot00 * dot11 - dot01 * dot01);
            Scalar u = (dot11 * dot02 - dot01 * dot12) * invDenom;
            Scalar v = (dot00 * dot12 - dot01 * dot02) * invDenom;

            if (u >= -Scalar(1e-10) && v >= -Scalar(1e-10) && (u + v) <= Scalar(1) + Scalar(1e-10)) {
                Scalar d = Point3::distance(point, proj);
                if (d < bestDist) { bestDist = d; bestPoint = proj; }
            }
        }

        return bestPoint;
    }

    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        Point3 apexPt = apex();
        Scalar halfW = m_baseWidth * Scalar(0.5);
        Scalar halfD = m_baseDepth * Scalar(0.5);

        Point3 corners[4] = {
            m_baseCenter - m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD,
            m_baseCenter + m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD,
            m_baseCenter - m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD
        };

        std::optional<Point3> bestHit;
        Scalar bestT = std::numeric_limits<Scalar>::max();

        Plane basePlane(m_baseCenter, m_apexDirection.opposite());
        if (auto optHit = basePlane.intersect(ray)) {
            Scalar t = Vector3::dot(*optHit - ray.origin, ray.direction) / ray.direction.lengthSquared();
            if (t > tolerance && t < bestT && contains(*optHit, tolerance)) {
                bestHit = optHit;
                bestT = t;
            }
        }

        for (int i = 0; i < 4; ++i) {
            Point3 a = corners[i];
            Point3 b = corners[(i + 1) % 4];
            Plane facePlane(a, Direction3::fromVector(Vector3::cross(b - a, apexPt - a)));

            if (auto optHit = facePlane.intersect(ray)) {
                Scalar t = Vector3::dot(*optHit - ray.origin, ray.direction) / ray.direction.lengthSquared();
                if (t > tolerance && t < bestT) {
                    Vector3 v0 = b - a;
                    Vector3 v1 = apexPt - a;
                    Vector3 v2 = *optHit - a;
                    Scalar dot00 = Vector3::dot(v0, v0);
                    Scalar dot01 = Vector3::dot(v0, v1);
                    Scalar dot02 = Vector3::dot(v0, v2);
                    Scalar dot11 = Vector3::dot(v1, v1);
                    Scalar dot12 = Vector3::dot(v1, v2);
                    Scalar invDenom = Scalar(1) / (dot00 * dot11 - dot01 * dot01);
                    Scalar u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                    Scalar v = (dot00 * dot12 - dot01 * dot02) * invDenom;

                    if (u >= -tolerance && v >= -tolerance && (u + v) <= Scalar(1) + tolerance) {
                        bestHit = optHit;
                        bestT = t;
                    }
                }
            }
        }

        return bestHit;
    }

    [[nodiscard]] AABB boundingBox() const override {
        AABB aabb;
        auto extendPoint = [&aabb](const Point3& p) {
            aabb.extend(Vector3(p.x, p.y, p.z));
        };

        extendPoint(m_baseCenter);
        extendPoint(apex());

        Scalar halfW = m_baseWidth * Scalar(0.5);
        Scalar halfD = m_baseDepth * Scalar(0.5);
        extendPoint(m_baseCenter - m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD);
        extendPoint(m_baseCenter + m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD);
        extendPoint(m_baseCenter + m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD);
        extendPoint(m_baseCenter - m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD);

        return aabb;
    }

    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newPyramid = std::make_unique<Pyramid>(*this);
        newPyramid->m_baseCenter = t.transformPoint(m_baseCenter);
        newPyramid->m_baseDirX = Direction3::fromVector(t.transformDirection(m_baseDirX.asVector()));
        newPyramid->m_baseDirY = Direction3::fromVector(t.transformDirection(m_baseDirY.asVector()));
        newPyramid->m_apexDirection = Direction3::fromVector(t.transformDirection(m_apexDirection.asVector()));
        return newPyramid;
    }

    [[nodiscard]] Solid::Mesh tessellate(int /*subdivisions*/ = 1) const override {
        Mesh mesh;

        Point3 apexPt = apex();
        Scalar halfW = m_baseWidth * Scalar(0.5);
        Scalar halfD = m_baseDepth * Scalar(0.5);

        mesh.vertices.push_back(m_baseCenter - m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD); // 0
        mesh.vertices.push_back(m_baseCenter + m_baseDirX.asVector() * halfW - m_baseDirY.asVector() * halfD); // 1
        mesh.vertices.push_back(m_baseCenter + m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD); // 2
        mesh.vertices.push_back(m_baseCenter - m_baseDirX.asVector() * halfW + m_baseDirY.asVector() * halfD); // 3
        mesh.vertices.push_back(apexPt); // 4

        mesh.faces = {
            {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4},  // боковые
            {0, 3, 2}, {0, 2, 1}  // основание
        };

        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Pyramid";
    }

private:
    Point3      m_baseCenter;
    Direction3  m_baseDirX;
    Scalar      m_baseWidth;
    Direction3  m_baseDirY;
    Scalar      m_baseDepth;
    Direction3  m_apexDirection;
    Scalar      m_height;
};

} // namespace mir