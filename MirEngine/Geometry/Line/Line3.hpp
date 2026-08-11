// MirEngine/Geometry/Line/Line3.hpp
// ➖ Прямая линия в трёхмерном пространстве — бесконечная прямая,
//    заданная точкой и направлением.
//
// Прямая (Line3) — это один из базовых элементов геометрии. Она не имеет
// начала и конца, тянется бесконечно в обе стороны. Чтобы задать прямую,
// достаточно указать любую точку на ней и направление, куда она идёт.
//
// Представь себе лазерный луч, который пронзает пространство насквозь:
// точка — это место, где луч выходит из указки, а направление — куда
// он светит. Луч бесконечен и вперёд, и назад.
//
// Line3 используется для:
//   • Построения осей вращения.
//   • Расчёта пересечений (линия с плоскостью, линия с линией).
//   • Определения нормалей к поверхностям.
//   • Создания направляющих для выдавливания и сдвига.
//   • Вычисления расстояний от точки до прямой.
//
// Важные методы:
//   • distanceTo(point) — кратчайшее расстояние от точки до прямой.
//   • project(point)    — проекция точки на прямую (ближайшая точка на прямой).
//   • closestPointsWith(other) — ближайшие точки между двумя прямыми.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar

namespace mir {

class Line3 {
public:
    // ── Компоненты ───────────────────────────────────────────
    Point3  origin;       // точка, через которую проходит прямая
    Vector3 direction;    // направление прямой (ненулевой вектор)

    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт прямую по точке и направлению.
    // Направление не обязано быть единичной длины, но не должно быть нулевым.
    constexpr Line3(const Point3& origin, const Vector3& direction) noexcept
        : origin(origin), direction(direction)
    {}

    // Создаёт прямую, проходящую через две точки.
    [[nodiscard]] static Line3 fromTwoPoints(const Point3& p1, const Point3& p2) noexcept {
        return Line3(p1, p2 - p1);
    }

    // ── Вспомогательные методы ───────────────────────────────

    // Проверяет, лежит ли точка на прямой (с заданным допуском).
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept {
        // Точка лежит на прямой, если вектор (point - origin) коллинеарен direction.
        // Это проверяется через векторное произведение: если оно нулевое, то коллинеарны.
        Vector3 cross = Vector3::cross(point - origin, direction);
        return cross.lengthSquared() <= tolerance * tolerance;
    }

    // ── Расстояние от точки до прямой ──────────────────────
    [[nodiscard]] Scalar distanceTo(const Point3& point) const noexcept {
        // Формула: |(point - origin) × direction| / |direction|
        Vector3 cross = Vector3::cross(point - origin, direction);
        return cross.length() / direction.length();
    }

    // ── Проекция точки на прямую ────────────────────────────
    // Возвращает точку на прямой, ближайшую к заданной.
    [[nodiscard]] Point3 project(const Point3& point) const noexcept {
        // Формула: origin + ((point - origin) · direction) / (direction · direction) * direction
        Vector3 v = point - origin;
        Scalar t = Vector3::dot(v, direction) / direction.lengthSquared();
        return origin + (direction * t);
    }

    // ── Параметрическая точка ────────────────────────────────
    // Возвращает точку на прямой на расстоянии t * |direction| от origin.
    // При t=0 возвращает origin, t=1 — origin + direction.
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept {
        return origin + (direction * t);
    }

    // ── Ближайшие точки между двумя прямыми ──────────────────
    // Возвращает пару точек (на этой прямой, на другой прямой),
    // расстояние между которыми минимально.
    [[nodiscard]] static std::pair<Point3, Point3> closestPoints(const Line3& a, const Line3& b) noexcept {
        // Алгоритм из "Real-Time Collision Detection" (Christer Ericson).
        Vector3 d1 = a.direction;
        Vector3 d2 = b.direction;
        Vector3 r  = a.origin - b.origin;

        Scalar a_dot = Vector3::dot(d1, d1);
        Scalar b_dot = Vector3::dot(d2, d2);
        Scalar ab_dot = Vector3::dot(d1, d2);
        Scalar r_dot_d1 = Vector3::dot(r, d1);
        Scalar r_dot_d2 = Vector3::dot(r, d2);

        Scalar denom = a_dot * b_dot - ab_dot * ab_dot;

        Scalar t1 = 0.0;
        Scalar t2 = 0.0;

        if (denom > 1e-20) {
            t1 = (ab_dot * r_dot_d2 - b_dot * r_dot_d1) / denom;
            t2 = (a_dot * r_dot_d2 - ab_dot * r_dot_d1) / denom;
        } else {
            // Прямые параллельны — берём начало a и его проекцию на b.
            t2 = r_dot_d2 / b_dot;
        }

        Point3 p1 = a.origin + (d1 * t1);
        Point3 p2 = b.origin + (d2 * t2);
        return {p1, p2};
    }

    // ── Расстояние между двумя прямыми ──────────────────────
    [[nodiscard]] static Scalar distanceBetween(const Line3& a, const Line3& b) noexcept {
        auto [p1, p2] = closestPoints(a, b);
        return Point3::distance(p1, p2);
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Line3& a, const Line3& b) noexcept {
        // Две прямые равны, если они коллинеарны и имеют общую точку.
        // Проверяем: векторы коллинеарны (кросс-продукт = 0) и origin b лежит на прямой a.
        Vector3 cross = Vector3::cross(a.direction, b.direction);
        if (cross.lengthSquared() > 1e-20) return false; // не параллельны
        return a.contains(b.origin);
    }
    friend constexpr bool operator!=(const Line3& a, const Line3& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir