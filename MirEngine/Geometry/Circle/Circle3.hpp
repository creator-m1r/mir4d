// MirEngine/Geometry/Circle/Circle3.hpp
// ⭕ Окружность в трёхмерном пространстве — все точки на заданном расстоянии
//    от центра, лежащие в одной плоскости.
//
// Окружность (Circle3) — это множество точек, равноудалённых от центра
// и лежащих в одной плоскости. Она задаётся тремя параметрами:
//   • center    — центр окружности (точка в пространстве).
//   • normal    — направление "вверх" от плоскости окружности (нормаль).
//   • radius    — расстояние от центра до любой точки окружности.
//
// В отличие от двумерной окружности, Circle3 может быть расположена
// под любым углом в пространстве — например, это может быть отверстие
// в боковой стенке детали, а не только в "полу".
//
// Circle3 используется для:
//   • Построения цилиндрических поверхностей.
//   • Создания дуг (Arc3) — части окружности.
//   • Проверки столкновений (например, "пересекает ли луч это отверстие?").
//   • Инженерных операций (фаски, скругления вдоль окружности).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../Direction/Direction3.hpp"   // mir::Direction3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include "../../Core/Types/Angle.hpp"    // mir::Angle
#include <cmath>

namespace mir {

class Circle3 {
public:
    // ── Компоненты ───────────────────────────────────────────
    Point3      center;   // центр окружности
    Direction3  normal;   // нормаль к плоскости окружности (единичная)
    Scalar      radius;   // радиус (должен быть > 0)

    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт окружность с заданным центром, нормалью и радиусом.
    // Радиус должен быть положительным.
    constexpr Circle3(const Point3& center, const Direction3& normal, Scalar radius) noexcept
        : center(center), normal(normal), radius(radius > 0 ? radius : 1.0)
    {}

    // Создаёт окружность, проходящую через три точки.
    // Точки не должны лежать на одной прямой.
    [[nodiscard]] static Circle3 fromThreePoints(const Point3& p1, const Point3& p2, const Point3& p3) noexcept {
        // Вычисляем плоскость через три точки
        Vector3 v1 = p2 - p1;
        Vector3 v2 = p3 - p1;
        Direction3 normal = Direction3::fromVector(Vector3::cross(v1, v2));

        // Находим центр описанной окружности через решение системы уравнений
        // (алгоритм из "Geometric Tools").
        Vector3 d1 = v1;
        Vector3 d2 = v2;
        Scalar d1Sq = d1.lengthSquared();
        Scalar d2Sq = d2.lengthSquared();
        Scalar cross = 2.0 * Vector3::dot(d1, d2);
        
        Scalar denom = 4.0 * d1Sq * d2Sq - cross * cross;
        if (denom < 1e-20) {
            // Точки почти на одной прямой — возвращаем окружность большого радиуса
            return Circle3(p1, normal, 1.0);
        }
        
        Scalar t1 = (2.0 * d2Sq * d1Sq - d2Sq * cross) / denom;
        Scalar t2 = (2.0 * d1Sq * d2Sq - d1Sq * cross) / denom;
        
        Point3 center = p1 + (d1 * t1) + (d2 * t2);
        Scalar radius = Point3::distance(center, p1);
        
        return Circle3(center, normal, radius);
    }

    // ── Геометрические свойства ─────────────────────────────

    // Длина окружности (периметр).
    [[nodiscard]] constexpr Scalar circumference() const noexcept {
        return 2.0 * 3.14159265358979323846 * radius;
    }

    // Площадь круга.
    [[nodiscard]] constexpr Scalar area() const noexcept {
        return 3.14159265358979323846 * radius * radius;
    }

    // Диаметр.
    [[nodiscard]] constexpr Scalar diameter() const noexcept {
        return 2.0 * radius;
    }

    // ── Точка на окружности ─────────────────────────────────

    // Возвращает точку на окружности под заданным углом.
    // Угол отсчитывается от произвольной, но фиксированной для данной окружности
    // оси X' в плоскости окружности (направление X' = directionFromNormal(normal)).
    [[nodiscard]] Point3 pointAtAngle(const Angle& angle) const noexcept {
        // Строим локальный базис в плоскости окружности
        Vector3 xAxis = getLocalXAxis();
        Vector3 yAxis = Vector3::cross(normal.asVector(), xAxis);
        
        Scalar cosA = std::cos(angle.radians());
        Scalar sinA = std::sin(angle.radians());
        
        return center + (xAxis * (radius * cosA)) + (yAxis * (radius * sinA));
    }

    // ── Проверка принадлежности точки окружности ─────────────
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept {
        // Точка должна лежать в той же плоскости и на расстоянии radius от центра
        Vector3 toPoint = point - center;
        Scalar distToPlane = std::abs(Vector3::dot(toPoint, normal.asVector()));
        if (distToPlane > tolerance) return false;
        
        Scalar distToCenter = toPoint.length();
        return std::abs(distToCenter - radius) <= tolerance;
    }

    // ── Ближайшая точка на окружности к заданной точке ──────
    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept {
        // Проецируем точку на плоскость окружности
        Vector3 toPoint = point - center;
        Scalar distAlongNormal = Vector3::dot(toPoint, normal.asVector());
        Vector3 projected = toPoint - normal.asVector() * distAlongNormal;
        
        // Если проекция в центре, возвращаем произвольную точку на окружности
        Scalar projLen = projected.length();
        if (projLen < 1e-20) {
            return pointAtAngle(Angle::radians(0));
        }
        
        // Нормализуем и умножаем на радиус
        return center + projected.normalized() * radius;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Circle3& a, const Circle3& b) noexcept {
        return a.center == b.center && a.normal == b.normal && a.radius == b.radius;
    }
    friend constexpr bool operator!=(const Circle3& a, const Circle3& b) noexcept {
        return !(a == b);
    }

private:
    // Возвращает локальную ось X в плоскости окружности.
    [[nodiscard]] Vector3 getLocalXAxis() const noexcept {
        // Используем вектор, ортогональный normal, как локальную ось X.
        // Для этого берём cross(normal, unitX) — если normal не параллелен unitX.
        Vector3 candidate = Vector3::cross(normal.asVector(), Vector3::unitX());
        if (candidate.lengthSquared() < 1e-10) {
            // normal почти параллелен X — используем unitY
            candidate = Vector3::cross(normal.asVector(), Vector3::unitY());
        }
        return candidate.normalized();
    }
};

} // namespace mir