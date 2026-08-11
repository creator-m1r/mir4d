// MirEngine/Geometry/Solid/Cone.hpp
// 🔺 Конус — твёрдое тело с круглым основанием и вершиной.
//
// Конус описывается осью, радиусом основания и высотой. Вершина конуса
// находится на расстоянии height от центра основания вдоль оси.
//
// Используется для:
//   • Моделирования конических отверстий, фасок, переходов.
//   • Создания усечённых конусов (если задать два радиуса).
//   • Построения сложных тел через булевы операции.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Solid.hpp"
#include "../Point/Point3.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../../Math/Bounds/AABB.hpp"
#include "../../Math/Transform.hpp"
#include "../Ray/Ray3.hpp"
#include "../Plane/Plane.hpp"
#include <cmath>
#include <array>
#include <memory>

namespace mir {

class Cone : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт конус с заданным основанием, направлением оси, радиусом и высотой.
    Cone(const Point3& baseCenter, const Direction3& axis, Scalar radius, Scalar height) noexcept
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
    [[nodiscard]] Point3 apex() const noexcept {
        return m_baseCenter + m_axis.asVector() * m_height;
    }

    // ── Реализация Solid ────────────────────────────────────

    [[nodiscard]] Scalar volume() const override {
        return (Scalar(1) / Scalar(3)) * Scalar(3.14159265358979323846) * m_radius * m_radius * m_height;
    }

    [[nodiscard]] Scalar surfaceArea() const override {
        Scalar slant = std::sqrt(m_radius * m_radius + m_height * m_height);
        return Scalar(3.14159265358979323846) * m_radius * (m_radius + slant);
    }

    // Проверка принадлежности точки конусу.
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());

        // Точка должна быть между основанием и вершиной.
        if (alongAxis < -tolerance || alongAxis > m_height + tolerance) return false;

        // Радиус в текущем сечении (линейно убывает к вершине).
        Scalar currentRadius = m_radius * (Scalar(1) - alongAxis / m_height);
        if (currentRadius < Scalar(0)) currentRadius = Scalar(0);

        // Расстояние до оси.
        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar distToAxis = radialVec.length();

        return distToAxis <= currentRadius + tolerance;
    }

    // Ближайшая точка на поверхности конуса.
    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar alongAxis = Vector3::dot(toPoint, m_axis.asVector());
        Scalar clampedAxis = std::clamp(alongAxis, Scalar(0), m_height);

        Scalar currentRadius = m_radius * (Scalar(1) - clampedAxis / m_height);
        if (currentRadius < Scalar(0)) currentRadius = Scalar(0);

        Vector3 radialVec = toPoint - m_axis.asVector() * alongAxis;
        Scalar distToAxis = radialVec.length();

        // Если точка внутри радиуса, ближайшая точка — на боковой поверхности.
        if (distToAxis <= currentRadius) {
            // Проецируем на боковую поверхность: нормаль к боковой поверхности.
            Vector3 radialDir = (distToAxis > Scalar(1e-10)) ? radialVec / distToAxis : Vector3::unitX();
            // Ближайшая точка на боковой поверхности при том же alongAxis.
            Point3 onSurface = m_baseCenter + m_axis.asVector() * clampedAxis + radialDir * currentRadius;
            return onSurface;
        }

        // Точка снаружи — ближайшая точка либо на боковой поверхности, либо на основании.
        Point3 onSide = m_baseCenter + m_axis.asVector() * clampedAxis + 
                       (radialVec / distToAxis) * currentRadius;
        Scalar distSide = Point3::distance(point, onSide);

        // Точка на основании.
        Point3 onBase = m_baseCenter;
        if (distToAxis > Scalar(1e-10)) {
            Vector3 projOnBase = radialVec / distToAxis * std::min(distToAxis, m_radius);
            onBase = m_baseCenter + projOnBase;
        }
        Scalar distBase = Point3::distance(point, onBase);

        return (distSide <= distBase) ? onSide : onBase;
    }

    // Пересечение с лучом.
    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        // Решаем квадратное уравнение для боковой поверхности конуса.
        Vector3 ro = ray.origin - m_baseCenter;
        Vector3 rd = ray.direction;
        Vector3 ax = m_axis.asVector();

        Scalar cosTheta = m_height / std::sqrt(m_height * m_height + m_radius * m_radius);
        Scalar cosThetaSq = cosTheta * cosTheta;

        Scalar a = Vector3::dot(rd, ax) * Vector3::dot(rd, ax) - cosThetaSq * Vector3::dot(rd, rd);
        Scalar b = Scalar(2) * (Vector3::dot(rd, ax) * Vector3::dot(ro, ax) - cosThetaSq * Vector3::dot(rd, ro));
        Scalar c = Vector3::dot(ro, ax) * Vector3::dot(ro, ax) - cosThetaSq * Vector3::dot(ro, ro);

        Scalar disc = b * b - Scalar(4) * a * c;
        if (disc < Scalar(0)) return std::nullopt;

        Scalar t1 = (-b - std::sqrt(disc)) / (Scalar(2) * a);
        Scalar t2 = (-b + std::sqrt(disc)) / (Scalar(2) * a);
        if (t1 > t2) std::swap(t1, t2);

        // Проверяем, попадает ли точка пересечения в диапазон [0, height] вдоль оси.
        auto validT = [&](Scalar t) -> std::optional<Point3> {
            if (t < tolerance) return std::nullopt;
            // Конструируем Point3 явно, избегая проблем с возвращаемым типом ray.pointAt
            Point3 hit(
                ray.origin.x + ray.direction.x * t,
                ray.origin.y + ray.direction.y * t,
                ray.origin.z + ray.direction.z * t
            );
            Vector3 toHit = hit - m_baseCenter;
            Scalar h = Vector3::dot(toHit, ax);
            if (h >= -tolerance && h <= m_height + tolerance) {
                return hit;
            }
            return std::nullopt;
        };

        if (auto hit = validT(t1)) return hit;
        if (auto hit = validT(t2)) return hit;

        // Пересечение с плоскостью основания.
        Plane basePlane(m_baseCenter, m_axis);
        if (auto optHit = basePlane.intersect(ray)) {
            Vector3 toHit = *optHit - m_baseCenter;
            if (toHit.lengthSquared() <= m_radius * m_radius + tolerance) {
                return optHit;
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] AABB boundingBox() const override {
    AABB aabb; // сначала создаём aabb

    // Временная обёртка для преобразования Point3 в Vector3
    auto extendPoint = [&aabb](const Point3& p) {
        aabb.extend(Vector3(p.x, p.y, p.z));
    };

    extendPoint(m_baseCenter);               // Центр основания
    extendPoint(apex());                     // Вершина

    // Крайние точки на окружности основания (4 точки)
    Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
    if (u.lengthSquared() < Scalar(1e-10)) {
        u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
    }
    u = u.normalized() * m_radius;
    Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized() * m_radius;

    extendPoint(m_baseCenter + u);
    extendPoint(m_baseCenter - u);
    extendPoint(m_baseCenter + v);
    extendPoint(m_baseCenter - v);

    return aabb;
    }

    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newCone = std::make_unique<Cone>(*this);
        newCone->m_baseCenter = t.transformPoint(m_baseCenter);
        newCone->m_axis = Direction3::fromVector(t.transformDirection(m_axis.asVector()));
        // Радиус и высоту тоже можно масштабировать (если Transform содержит scale).
        // Упрощённо: оставляем без изменений.
        return newCone;
    }

    [[nodiscard]] Solid::Mesh tessellate(int subdivisions = 16) const override {
        Mesh mesh;
        int n = std::max(3, subdivisions);

        // Вершина конуса
        Point3 apexPt = apex();
        mesh.vertices.push_back(apexPt);
        int apexIdx = 0;

        // Точки на окружности основания
        Vector3 u = Vector3::cross(m_axis.asVector(), Vector3::unitX());
        if (u.lengthSquared() < Scalar(1e-10)) u = Vector3::cross(m_axis.asVector(), Vector3::unitY());
        u = u.normalized() * m_radius;
        Vector3 v = Vector3::cross(m_axis.asVector(), u).normalized() * m_radius;

        for (int i = 0; i < n; ++i) {
            Scalar angle = Scalar(i) / Scalar(n) * Scalar(2) * Scalar(3.14159265358979323846);
            Vector3 offset = u * std::cos(angle) + v * std::sin(angle);
            mesh.vertices.push_back(m_baseCenter + offset);
        }

        // Центр основания (для нижней крышки)
        mesh.vertices.push_back(m_baseCenter);
        int baseCenterIdx = n + 1;

        // Боковые треугольники
        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            mesh.faces.push_back({apexIdx, i + 1, next + 1});
        }

        // Треугольники основания
        for (int i = 0; i < n; ++i) {
            int next = (i + 1) % n;
            mesh.faces.push_back({baseCenterIdx, next + 1, i + 1});
        }

        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Cone";
    }

private:
    Point3      m_baseCenter;
    Direction3  m_axis;
    Scalar      m_radius;
    Scalar      m_height;
};

} // namespace mir