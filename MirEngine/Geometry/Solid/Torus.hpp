// MirEngine/Geometry/Solid/Torus.hpp
// 🍩 Тор (тороид) — твёрдое тело в форме бублика.
//
// Тор описывается центром, направлением оси и двумя радиусами:
//   • majorRadius — расстояние от центра до оси тора (радиус "кольца").
//   • minorRadius — радиус самого "бублика" (толщина кольца).
//
// Тор широко используется в машиностроении:
//   • Уплотнительные кольца, подшипники, муфты.
//   • Декоративные элементы (ручки, ободки).
//   • Трубопроводная арматура (колена, переходы).
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
#include <cmath>
#include <array>
#include <memory>

namespace mir {

class Torus : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    Torus(const Point3& center, const Direction3& axis,
          Scalar majorRadius, Scalar minorRadius) noexcept
        : m_center(center)
        , m_axis(axis)
        , m_majorRadius(std::abs(majorRadius))
        , m_minorRadius(std::abs(minorRadius))
    {}

    // ── Доступ к параметрам ─────────────────────────────────
    [[nodiscard]] Point3     center()      const noexcept { return m_center; }
    [[nodiscard]] Direction3 axis()        const noexcept { return m_axis; }
    [[nodiscard]] Scalar     majorRadius() const noexcept { return m_majorRadius; }
    [[nodiscard]] Scalar     minorRadius() const noexcept { return m_minorRadius; }

    // ── Реализация Solid ────────────────────────────────────

    [[nodiscard]] Scalar volume() const override {
        return Scalar(2) * Scalar(3.14159265358979323846) * Scalar(3.14159265358979323846)
             * m_majorRadius * m_minorRadius * m_minorRadius;
    }

    [[nodiscard]] Scalar surfaceArea() const override {
        return Scalar(4) * Scalar(3.14159265358979323846) * Scalar(3.14159265358979323846)
             * m_majorRadius * m_minorRadius;
    }

    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 toPoint = point - m_center;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());

        if (std::abs(alongAxis) > m_minorRadius + tolerance) return false;

        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar radialDist = radialVec.length();

        Scalar distToCircle = std::abs(radialDist - m_majorRadius);
        Scalar distToCenter = std::sqrt(distToCircle * distToCircle + alongAxis * alongAxis);

        return distToCenter <= m_minorRadius + tolerance;
    }

    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        Vector3 toPoint = point - m_center;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());
        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar radialDist = radialVec.length();

        Vector3 radialDir = (radialDist > Scalar(1e-10)) ? radialVec / radialDist : Vector3::unitX();
        Point3 circleCenter = m_center + radialDir * m_majorRadius;

        Vector3 toCircleCenter = point - circleCenter;
        Scalar distToCircleCenter = toCircleCenter.length();
        if (distToCircleCenter < Scalar(1e-10)) {
            return circleCenter + m_axis.asVector().normalized() * m_minorRadius;
        }

        Vector3 dirToSurface = toCircleCenter / distToCircleCenter;
        return circleCenter + dirToSurface * m_minorRadius;
    }

    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        // Итеративный поиск пересечения с уточнением бинарным поиском.
        Scalar t = tolerance;
        Scalar step = m_majorRadius * Scalar(0.02);
        for (int i = 0; i < 200; ++i) {
            // Явное конструирование Point3, не полагаясь на ray.pointAt (может возвращать Vector3)
            Point3 p = ray.origin + ray.direction * t;
            if (contains(p, tolerance * Scalar(10))) {
                Scalar tIn = t - step * Scalar(2);
                Scalar tOut = t;
                for (int j = 0; j < 20; ++j) {
                    Scalar tMid = (tIn + tOut) * Scalar(0.5);
                    Point3 midPt = ray.origin + ray.direction * tMid;
                    if (contains(midPt, tolerance)) {
                        tOut = tMid;
                    } else {
                        tIn = tMid;
                    }
                }
                return Point3(ray.origin + ray.direction * tOut);
            }
            t += step;
            if (t > m_majorRadius * Scalar(10)) break;
        }
        return std::nullopt;
    }

    [[nodiscard]] AABB boundingBox() const override {
        Scalar outerRadius = m_majorRadius + m_minorRadius;
        Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
        if (u.lengthSquared() < Scalar(1e-10)) u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
        u = u.normalized() * outerRadius;
        Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized() * outerRadius;
        Vector3 w = m_axis.asVector() * m_minorRadius;

        AABB aabb;
        // Лямбда для безопасного расширения AABB (преобразует Point3 -> Vector3)
        auto extendPoint = [&aabb](const Point3& pt) {
            aabb.extend(Vector3(pt.x, pt.y, pt.z));
        };

        extendPoint(m_center + u + w);
        extendPoint(m_center + u - w);
        extendPoint(m_center - u + w);
        extendPoint(m_center - u - w);
        extendPoint(m_center + v + w);
        extendPoint(m_center + v - w);
        extendPoint(m_center - v + w);
        extendPoint(m_center - v - w);
        return aabb;
    }

    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newTorus = std::make_unique<Torus>(*this);
        newTorus->m_center = t.transformPoint(m_center);
        newTorus->m_axis = Direction3::fromVector(t.transformDirection(m_axis.asVector()));
        return newTorus;
    }

    [[nodiscard]] Solid::Mesh tessellate(int subdivisions = 20) const override {
        Mesh mesh;
        int n = std::max(4, subdivisions);
        int m = std::max(4, subdivisions / 2);

        Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
        if (u.lengthSquared() < Scalar(1e-10)) u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
        u = u.normalized();
        Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized();

        for (int i = 0; i < n; ++i) {
            Scalar theta = Scalar(i) / Scalar(n) * Scalar(2) * Scalar(3.14159265358979323846);
            Vector3 circleCenter = u * (m_majorRadius * std::cos(theta)) +
                                   v * (m_majorRadius * std::sin(theta));
            Point3 centerOnCircle = m_center + circleCenter;

            Vector3 radialDir = circleCenter / m_majorRadius;
            Vector3 axisVec = m_axis.asVector();

            for (int j = 0; j < m; ++j) {
                Scalar phi = Scalar(j) / Scalar(m) * Scalar(2) * Scalar(3.14159265358979323846);
                Vector3 offset = radialDir * (m_minorRadius * std::cos(phi)) +
                                 axisVec * (m_minorRadius * std::sin(phi));
                mesh.vertices.push_back(centerOnCircle + offset);
            }
        }

        for (int i = 0; i < n; ++i) {
            int iNext = (i + 1) % n;
            for (int j = 0; j < m; ++j) {
                int jNext = (j + 1) % m;
                int v00 = i * m + j;
                int v10 = iNext * m + j;
                int v01 = i * m + jNext;
                int v11 = iNext * m + jNext;

                mesh.faces.push_back({v00, v10, v11});
                mesh.faces.push_back({v00, v11, v01});
            }
        }

        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Torus";
    }

private:
    Point3      m_center;
    Direction3  m_axis;
    Scalar      m_majorRadius;
    Scalar      m_minorRadius;
};

} // namespace mir