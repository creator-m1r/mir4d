// MirEngine/Geometry/Ray/Ray3.hpp
// ➡️ Луч в трёхмерном пространстве — полубесконечная линия с началом.
//
// Луч (Ray) — это как прямая линия, но только в одну сторону от начальной
// точки. У него есть точка старта (origin) и направление (direction).
// Луч бесконечно продолжается только ВПЕРЁД по направлению, а назад —
// обрывается в точке старта.
//
// Представь фонарик: свет выходит из лампочки (origin) и распространяется
// вперёд (direction), но не назад.
//
// Луч используется для:
//   • Трассировки лучей (ray tracing) — симуляции света.
//   • Проверки видимости (что видно из данной точки?).
//   • Выделения объектов мышкой (ray casting).
//   • Расчёта столкновений (например, снаряд летит из точки А в направлении В).
//
// Отличие от Line3:
//   Line3 — бесконечна в обе стороны.
//   Ray3 — бесконечна только вперёд (t >= 0).
//   Segment3 (будет позже) — конечен, от точки А до точки Б (0 <= t <= 1).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <optional>
#include <cmath>

namespace mir {

class Ray3 {
public:
    // ── Компоненты ───────────────────────────────────────────
    Point3  origin;       // начальная точка луча
    Vector3 direction;    // направление (ненулевой вектор)

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Ray3(const Point3& origin, const Vector3& direction) noexcept
        : origin(origin), direction(direction)
    {}

    // ── Параметрическая точка ────────────────────────────────
    // Возвращает точку на луче: origin + t * direction.
    // t = 0 → origin, t < 0 — не принадлежит лучу.
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept {
        return origin + (direction * t);
    }

    // ── Ближайшая точка на луче ──────────────────────────────
    [[nodiscard]] Point3 closestPoint(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        Vector3 v = point - origin;
        Scalar dot = Vector3::dot(v, direction);
        if (dot <= tolerance) {
            return origin;   // проекция позади или на начале луча
        }
        Scalar lenSq = direction.lengthSquared();
        if (lenSq < tolerance * tolerance) {
            return origin;   // направление нулевое
        }
        Scalar t = dot / lenSq;
        return origin + (direction * t);
    }

    // ── Расстояние от точки до луча ──────────────────────────
    [[nodiscard]] Scalar distanceTo(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        return Point3::distance(point, closestPoint(point, tolerance));
    }

    // ── Проверка принадлежности точки лучу ──────────────────
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        Vector3 v = point - origin;
        // Проверяем коллинеарность через векторное произведение
        Vector3 cross = Vector3::cross(v, direction);
        if (cross.lengthSquared() > tolerance * tolerance) {
            return false;
        }
        // Проверяем, что точка лежит вперёд (t >= 0)
        Scalar t = Vector3::dot(v, direction);
        return t >= -tolerance;
    }

    // ── Пересечение с плоскостью, заданной точкой и нормалью ───
    // Возвращает точку пересечения, если луч направлен в сторону плоскости.
    [[nodiscard]] std::optional<Point3> intersectPlane(const Point3& planePoint,
                                                       const Vector3& planeNormal,
                                                       Scalar tolerance = Scalar(1e-10)) const noexcept {
        Scalar denom = Vector3::dot(direction, planeNormal);
        if (std::abs(denom) < tolerance) {
            return std::nullopt;   // луч параллелен плоскости
        }
        Scalar t = Vector3::dot(planePoint - origin, planeNormal) / denom;
        if (t >= tolerance) {
            return pointAt(t);
        }
        return std::nullopt;   // пересечение позади луча
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Ray3& a, const Ray3& b) noexcept {
        return a.origin == b.origin && a.direction == b.direction;
    }
    friend constexpr bool operator!=(const Ray3& a, const Ray3& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir