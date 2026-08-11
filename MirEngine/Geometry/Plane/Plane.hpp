// MirEngine/Geometry/Plane/Plane.hpp
// 📐 Плоскость в трёхмерном пространстве — бесконечная поверхность,
//    заданная точкой и направлением нормали.
//
// Плоскость — это как идеально ровный лист бумаги, который простирается
// бесконечно во все стороны. Чтобы задать его положение в пространстве,
// нужны две вещи:
//   1. Любая точка на этом листе (origin).
//   2. Направление "вверх" от листа (normal) — перпендикулярный вектор.
//
// Нормаль — это вектор, который "торчит" из плоскости под прямым углом.
// Он определяет, куда смотрит плоскость. Например, пол имеет нормаль,
// направленную вверх (0,0,1), а стена — в сторону (1,0,0).
//
// Плоскость используется для:
//   • Отсечения невидимых поверхностей (фrustum culling).
//   • Расчёта столкновений и отражений.
//   • Построения сечений 3D-объектов.
//   • Определения "верхней" и "нижней" стороны поверхности.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"            // mir::Point3
#include "../Direction/Direction3.hpp"     // mir::Direction3
#include "../../Math/Vector/Vector3.hpp"   // mir::Vector3
#include "../Line/Line3.hpp"              // mir::Line3
#include "../Ray/Ray3.hpp"                // mir::Ray3
#include "../Segment/Segment3.hpp"        // mir::Segment3
#include "../../Core/Types/Scalar.hpp"     // mir::Scalar
#include <optional>                        // std::optional
#include <cmath>                           // std::abs

namespace mir {

class Plane {
public:
    // ── Компоненты ───────────────────────────────────────────
    Point3     origin;    // любая точка на плоскости
    Direction3 normal;    // нормаль (перпендикуляр), гарантированно единичной длины

    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт плоскость по точке и нормали.
    // Direction3 автоматически обеспечивает единичную длину при создании.
    constexpr Plane(const Point3& origin, const Direction3& normal) noexcept
        : origin(origin), normal(normal)
    {}

    // Создаёт плоскость по трём точкам (не лежащим на одной прямой).
    [[nodiscard]] static Plane fromThreePoints(const Point3& p1, const Point3& p2, const Point3& p3) noexcept {
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;
        Direction3 n = Direction3::fromVector(Vector3::cross(v1, v2));
        return Plane(p1, n);
    }

    // ── Нормализация ─────────────────────────────────────────
    // Возвращает плоскость с единичной нормалью (уже гарантирована типом Direction3).
    [[nodiscard]] Plane normalized() const noexcept {
        return *this;
    }

    // ── Расстояние от точки до плоскости ─────────────────────
    // Положительное значение означает, что точка находится с той же стороны,
    // куда смотрит нормаль; отрицательное — с противоположной.
    [[nodiscard]] Scalar signedDistance(const Point3& point) const noexcept {
        Vector3 v = point - origin;
        return Vector3::dot(v, normal.asVector());
    }

    // Абсолютное расстояние (всегда ≥ 0).
    [[nodiscard]] Scalar distance(const Point3& point) const noexcept {
        return std::abs(signedDistance(point));
    }

    // ── Проекция точки на плоскость ──────────────────────────
    // Возвращает точку на плоскости, ближайшую к заданной.
    [[nodiscard]] Point3 project(const Point3& point) const noexcept {
        Scalar dist = signedDistance(point);
        return point - normal.asVector() * dist;
    }

    // ── Проверка принадлежности точки плоскости ──────────────
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept {
        return distance(point) <= tolerance;
    }

    // ── Пересечение с прямой (Line3) ─────────────────────────
    // Возвращает точку пересечения или ничего, если прямая параллельна плоскости.
    [[nodiscard]] std::optional<Point3> intersect(const Line3& line) const noexcept {
        Vector3 dir = line.direction;
        Scalar denom = Vector3::dot(dir, normal.asVector());

        if (std::abs(denom) < 1e-20) {
            return std::nullopt;   // прямая параллельна плоскости
        }

        Scalar t = Vector3::dot(origin - line.origin, normal.asVector()) / denom;
        return line.pointAt(t);
    }

    // ── Пересечение с лучом (Ray3) ──────────────────────────
    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray) const noexcept {
        Scalar denom = Vector3::dot(ray.direction, normal.asVector());
        if (std::abs(denom) < 1e-20) {
            return std::nullopt;   // луч параллелен плоскости
        }
        Scalar t = Vector3::dot(origin - ray.origin, normal.asVector()) / denom;
        if (t >= 0.0) {
            return ray.pointAt(t);
        }
        return std::nullopt;   // пересечение позади луча
    }

    // ── Пересечение с отрезком (Segment3) ────────────────────
    [[nodiscard]] std::optional<Point3> intersect(const Segment3& segment) const noexcept {
        Scalar d1 = signedDistance(segment.start);
        Scalar d2 = signedDistance(segment.end);

        if (std::abs(d1 - d2) < 1e-20) {
            // отрезок параллелен плоскости
            if (std::abs(d1) < 1e-20) {
                return segment.start;   // лежит в плоскости
            }
            return std::nullopt;
        }

        Scalar t = d1 / (d1 - d2);
        if (t >= 0.0 && t <= 1.0) {
            return segment.pointAt(t);
        }
        return std::nullopt;   // пересечение за пределами отрезка
    }

    // ── С какой стороны от плоскости находится точка? ────────
    [[nodiscard]] int sideOf(const Point3& point, Scalar tolerance = 1e-10) const noexcept {
        Scalar d = signedDistance(point);
        if (d > tolerance)  return 1;    // спереди (по направлению нормали)
        if (d < -tolerance) return -1;   // сзади
        return 0;                         // на плоскости
    }

    // ── Отражение точки относительно плоскости ───────────────
    [[nodiscard]] Point3 mirror(const Point3& point) const noexcept {
        Scalar d = signedDistance(point);
        return point - normal.asVector() * 2.0 * d;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Plane& a, const Plane& b) noexcept {
        // Две плоскости равны, если их нормали коллинеарны и точка одной лежит в другой.
        Vector3 cross = Vector3::cross(a.normal.asVector(), b.normal.asVector());
        if (cross.lengthSquared() > 1e-20) return false; // нормали не параллельны
        return a.contains(b.origin);
    }
    friend constexpr bool operator!=(const Plane& a, const Plane& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir