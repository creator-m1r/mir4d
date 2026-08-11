// MirEngine/Geometry/Solid/Cylinder.hpp
// 🥫 Цилиндр — твёрдое тело с круглым основанием, вытянутое вдоль оси.
//
// Цилиндр описывается центром основания, направлением оси, радиусом и высотой.
// Все точки, расстояние от которых до оси не превышает radius и проекция на ось
// лежит в интервале [0, height], принадлежат цилиндру.
//
// Используется для:
//   • Моделирования валов, отверстий, труб.
//   • Создания усечённых цилиндров и фасок через булевы операции.
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
#include <algorithm>

namespace mir {

class Cylinder : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт цилиндр с основанием в начале координат, осью Z, радиусом и высотой.
    Cylinder(Scalar radius, Scalar height) noexcept
        : m_baseCenter(Point3::origin())
        , m_axis(Direction3::unitZ())
        , m_radius(std::abs(radius))
        , m_height(std::abs(height))
    {}

    // Создаёт цилиндр с заданным основанием, осью, радиусом и высотой.
    Cylinder(const Point3& baseCenter, const Direction3& axis,
             Scalar radius, Scalar height) noexcept
        : m_baseCenter(baseCenter)
        , m_axis(axis)
        , m_radius(std::abs(radius))
        , m_height(std::abs(height))
    {}

    // ── Доступ к параметрам ─────────────────────────────────
    [[nodiscard]] Point3 baseCenter() const noexcept { return m_baseCenter; }
    [[nodiscard]] Direction3 axis() const noexcept { return m_axis; }
    [[nodiscard]] Scalar radius() const noexcept { return m_radius; }
    [[nodiscard]] Scalar height() const noexcept { return m_height; }
    [[nodiscard]] Point3 topCenter() const noexcept {
        return m_baseCenter + m_axis.asVector() * m_height;
    }

    // ── Реализация Solid ────────────────────────────────────

    // Объём цилиндра = π × r² × h
    [[nodiscard]] Scalar volume() const override {
        return Scalar(3.14159265358979323846) * m_radius * m_radius * m_height;
    }

    // Площадь поверхности = 2πr(r + h)
    [[nodiscard]] Scalar surfaceArea() const override {
        return Scalar(2) * Scalar(3.14159265358979323846) * m_radius * (m_radius + m_height);
    }

    // Проверка принадлежности точки цилиндру.
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());

        // Точка должна быть между основанием и вершиной.
        if (alongAxis < -tolerance || alongAxis > m_height + tolerance) return false;

        // Расстояние до оси.
        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar distToAxis = radialVec.length();

        return distToAxis <= m_radius + tolerance;
    }

    // Ближайшая точка на поверхности цилиндра к заданной точке.
    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());
        Scalar clampedAxis = std::clamp(alongAxis, Scalar(0), m_height);

        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar distToAxis = radialVec.length();

        // Точка на оси (или очень близко).
        if (distToAxis < Scalar(1e-20)) {
            // Возвращаем точку на боковой поверхности на той же высоте.
            return m_baseCenter + m_axis.asVector() * clampedAxis + Vector3::unitX() * m_radius;
        }

        Vector3 radialDir = radialVec / distToAxis;

        // Если точка внутри радиуса, ближайшая точка — на боковой поверхности.
        if (distToAxis <= m_radius) {
            return m_baseCenter + m_axis.asVector() * clampedAxis + radialDir * m_radius;
        }

        // Точка снаружи — ближайшая точка либо на боковой поверхности, либо на основании/вершине.
        Point3 onSide = m_baseCenter + m_axis.asVector() * clampedAxis + radialDir * m_radius;
        Scalar distSide = Point3::distance(point, onSide);

        // Точка на основании.
        Point3 onBase = m_baseCenter + radialDir * std::min(distToAxis, m_radius);
        Scalar distBase = Point3::distance(point, onBase);

        // Точка на вершине.
        Point3 topCenter = m_baseCenter + m_axis.asVector() * m_height;
        Point3 onTop = topCenter + radialDir * std::min(distToAxis, m_radius);
        Scalar distTop = Point3::distance(point, onTop);

        if (distSide <= distBase && distSide <= distTop) return onSide;
        if (distBase <= distTop) return onBase;
        return onTop;
    }

    // Пересечение с лучом.
    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        // 1. Пересечение с боковой поверхностью (бесконечный цилиндр).
        Vector3 ro = ray.origin - m_baseCenter;
        Vector3 rd = ray.direction;
        Vector3 ax = m_axis.asVector();

        // Проекция на плоскость, перпендикулярную оси.
        Vector3 ro_perp = ro - ax * Vector3::dot(ro, ax);
        Vector3 rd_perp = rd - ax * Vector3::dot(rd, ax);

        Scalar a = Vector3::dot(rd_perp, rd_perp);
        Scalar b = Scalar(2) * Vector3::dot(rd_perp, ro_perp);
        Scalar c = Vector3::dot(ro_perp, ro_perp) - m_radius * m_radius;

        std::optional<Point3> bestHit;
        Scalar bestT = std::numeric_limits<Scalar>::max();

        auto tryT = [&](Scalar t) -> std::optional<Point3> {
            if (t < tolerance) return std::nullopt;
            // Явное конструирование Point3, избегая потенциальных проблем с ray.pointAt
            Point3 hit(
                ray.origin.x + ray.direction.x * t,
                ray.origin.y + ray.direction.y * t,
                ray.origin.z + ray.direction.z * t
            );
            Scalar h = Vector3::dot(hit - m_baseCenter, ax);
            if (h >= -tolerance && h <= m_height + tolerance) {
                return hit;
            }
            return std::nullopt;
        };

        if (a > Scalar(1e-20)) {
            Scalar disc = b * b - Scalar(4) * a * c;
            if (disc >= Scalar(0)) {
                Scalar sqrtDisc = std::sqrt(disc);
                Scalar t1 = (-b - sqrtDisc) / (Scalar(2) * a);
                Scalar t2 = (-b + sqrtDisc) / (Scalar(2) * a);
                if (t1 > t2) std::swap(t1, t2);

                if (auto hit = tryT(t1)) { bestHit = hit; bestT = t1; }
                if (auto hit = tryT(t2)) {
                    Scalar t2Val = (t2 > 0) ? t2 : 0;
                    if (t2Val < bestT) { bestHit = hit; bestT = t2Val; }
                }
            }
        }

        // 2. Пересечение с плоскостью основания.
        Plane basePlane(m_baseCenter, m_axis.opposite());
        if (auto optHit = basePlane.intersect(ray)) {
            Scalar t = Vector3::dot(*optHit - ray.origin, ray.direction) / ray.direction.lengthSquared();
            if (t > tolerance && t < bestT) {
                Vector3 toHit = *optHit - m_baseCenter;
                if (toHit.lengthSquared() <= m_radius * m_radius + tolerance * tolerance) {
                    bestHit = optHit;
                    bestT = t;
                }
            }
        }

        // 3. Пересечение с плоскостью вершины.
        Plane topPlane(topCenter(), m_axis);
        if (auto optHit = topPlane.intersect(ray)) {
            Scalar t = Vector3::dot(*optHit - ray.origin, ray.direction) / ray.direction.lengthSquared();
            if (t > tolerance && t < bestT) {
                Vector3 toHit = *optHit - topCenter();
                if (toHit.lengthSquared() <= m_radius * m_radius + tolerance * tolerance) {
                    bestHit = optHit;
                }
            }
        }

        return bestHit;
    }

    // Ограничивающая рамка (AABB).
    [[nodiscard]] AABB boundingBox() const override {
        Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
        if (u.lengthSquared() < Scalar(1e-10)) u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
        u = u.normalized() * m_radius;
        Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized() * m_radius;

        Point3 top = topCenter();
        AABB aabb;

        // Локальная функция для добавления точки в AABB с преобразованием Point3 -> Vector3
        auto extendPoint = [&aabb](const Point3& p) {
            aabb.extend(Vector3(p.x, p.y, p.z));
        };

        for (Scalar su = -Scalar(1); su <= Scalar(1); su += Scalar(2)) {
            for (Scalar sv = -Scalar(1); sv <= Scalar(1); sv += Scalar(2)) {
                extendPoint(m_baseCenter + u * su + v * sv);
                extendPoint(top + u * su + v * sv);
            }
        }
        return aabb;
    }

    // Трансформация цилиндра.
    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newCyl = std::make_unique<Cylinder>(*this);
        newCyl->m_baseCenter = t.transformPoint(m_baseCenter);
        newCyl->m_axis = Direction3::fromVector(t.transformDirection(m_axis.asVector()));
        // Масштабируем радиус и высоту по среднему масштабу по перпендикулярным направлениям.
        // Упрощённо: используем максимальный масштаб.
        Scalar maxScale = std::max({t.scale.x, t.scale.y, t.scale.z});
        newCyl->m_radius = m_radius * std::abs(maxScale);
        newCyl->m_height = m_height * std::abs(maxScale);
        return newCyl;
    }

    // Тесселяция цилиндра (сетка треугольников).
    [[nodiscard]] Solid::Mesh tessellate(int subdivisions = 20) const override {
        Mesh mesh;
        int n = std::max(3, subdivisions);   // число сегментов по окружности

        Point3 top = topCenter();

        // Боковая поверхность (вершины).
        Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
        if (u.lengthSquared() < Scalar(1e-10)) u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
        u = u.normalized() * m_radius;
        Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized() * m_radius;

        for (int i = 0; i < n; ++i) {
            Scalar angle = Scalar(i) / Scalar(n) * Scalar(2) * Scalar(3.14159265358979323846);
            Vector3 offset = u * std::cos(angle) + v * std::sin(angle);
            mesh.vertices.push_back(m_baseCenter + offset);   // нижнее кольцо (индексы 0..n-1)
        }
        for (int i = 0; i < n; ++i) {
            Scalar angle = Scalar(i) / Scalar(n) * Scalar(2) * Scalar(3.14159265358979323846);
            Vector3 offset = u * std::cos(angle) + v * std::sin(angle);
            mesh.vertices.push_back(top + offset);            // верхнее кольцо (индексы n..2n-1)
        }

        // Центры оснований.
        mesh.vertices.push_back(m_baseCenter);   // индекс 2n
        mesh.vertices.push_back(top);            // индекс 2n+1

        // Боковые треугольники.
        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            int v00 = i;
            int v10 = next;
            int v01 = n + i;
            int v11 = n + next;
            mesh.faces.push_back({v00, v10, v11});
            mesh.faces.push_back({v00, v11, v01});
        }

        // Треугольники оснований.
        int baseCenterIdx = 2 * n;
        int topCenterIdx = 2 * n + 1;
        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            mesh.faces.push_back({baseCenterIdx, next, i});          // нижнее
            mesh.faces.push_back({topCenterIdx, n + i, n + next});   // верхнее
        }

        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Cylinder";
    }

private:
    Point3      m_baseCenter;
    Direction3  m_axis;
    Scalar      m_radius;
    Scalar      m_height;
};

} // namespace mir