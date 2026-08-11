// MirEngine/Geometry/Segment/Segment3.hpp
// 📏 Отрезок в трёхмерном пространстве — конечная линия между двумя точками.
//
// Отрезок (Segment3) — это часть прямой, ограниченная двумя точками:
// началом (start) и концом (end). В отличие от прямой (Line3), которая
// бесконечна, и луча (Ray3), который бесконечен только вперёд, отрезок
// имеет чёткие границы. Это самый "осязаемый" линейный примитив:
// именно отрезками мы представляем рёбра деталей, грани меша, стороны
// прямоугольников.
//
// Отрезок используется для:
//   • Построения контуров (грани детали).
//   • Проверки пересечений с другими отрезками, лучами, плоскостями.
//   • Расчёта длин рёбер и периметров.
//   • Поиска ближайшей точки на отрезке (для привязки, коллизий).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <cmath>
#include <algorithm>
#include <optional>

namespace mir {

class Segment3 {
public:
    // ── Концы отрезка ────────────────────────────────────────
    Point3 start;
    Point3 end;

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Segment3() noexcept = default;

    constexpr Segment3(const Point3& start, const Point3& end) noexcept
        : start(start), end(end)
    {}

    // ── Геометрические свойства ──────────────────────────────
    [[nodiscard]] Vector3 vector() const noexcept {
        return end - start;
    }

    [[nodiscard]] Vector3 direction() const noexcept {
        Vector3 v = vector();
        Scalar len = v.length();
        return (len > Scalar(1e-20)) ? v / len : Vector3::zero();
    }

    [[nodiscard]] Scalar length() const noexcept {
        return vector().length();
    }

    [[nodiscard]] Scalar lengthSquared() const noexcept {
        return vector().lengthSquared();
    }

    [[nodiscard]] Point3 center() const noexcept {
        return Point3{
            (start.x + end.x) * Scalar(0.5),
            (start.y + end.y) * Scalar(0.5),
            (start.z + end.z) * Scalar(0.5)
        };
    }

    // ── Параметрическая точка ────────────────────────────────
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept {
        return start + vector() * t;
    }

    // ── Проверка принадлежности точки отрезку ────────────────
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        Vector3 v = point - start;
        Vector3 dir = vector();
        Scalar lenSq = dir.lengthSquared();
        if (lenSq < Scalar(1e-20)) {
            return v.lengthSquared() <= tolerance * tolerance;
        }
        Scalar t = Vector3::dot(v, dir) / lenSq;
        if (t < -tolerance || t > Scalar(1) + tolerance) {
            return false;
        }
        Point3 proj = pointAt(t);
        return Point3::distance(point, proj) <= tolerance;
    }

    // ── Ближайшая точка на отрезке ───────────────────────────
    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept {
        Vector3 dir = vector();
        Scalar lenSq = dir.lengthSquared();
        if (lenSq < Scalar(1e-20)) {
            return start;
        }
        Scalar t = Vector3::dot(point - start, dir) / lenSq;
        t = std::clamp(t, Scalar(0), Scalar(1));
        return pointAt(t);
    }

    // ── Расстояние от точки до отрезка ───────────────────────
    [[nodiscard]] Scalar distanceTo(const Point3& point) const noexcept {
        return Point3::distance(point, closestPoint(point));
    }

    // ── Пересечение с другим отрезком ────────────────────────
    [[nodiscard]] std::optional<Point3> intersect(const Segment3& other, Scalar tolerance = Scalar(1e-10)) const noexcept {
        // Вспомогательная функция: поиск ближайших точек на двух прямых
        auto closestPointsBetweenLines = [](
            const Point3& p1, const Vector3& d1,
            const Point3& p2, const Vector3& d2) -> std::pair<Point3, Point3>
        {
            Vector3 r = p1 - p2;
            Scalar a = Vector3::dot(d1, d1);
            Scalar e = Vector3::dot(d2, d2);
            Scalar f = Vector3::dot(d2, r);

            if (a < Scalar(1e-20) && e < Scalar(1e-20)) {
                // Обе прямые вырождены в точки
                return {p1, p2};
            }
            if (a < Scalar(1e-20)) {
                // d1 вырожден, d2 нет
                Scalar t2 = f / e;
                return {p1, p2 + d2 * t2};
            }
            if (e < Scalar(1e-20)) {
                // d2 вырожден, d1 нет
                Scalar t1 = -Vector3::dot(d1, r) / a;
                return {p1 + d1 * t1, p2};
            }
            Scalar c = Vector3::dot(d1, r);
            Scalar b = Vector3::dot(d1, d2);
            Scalar denom = a * e - b * b;
            if (std::abs(denom) < Scalar(1e-20)) {
                // Параллельные прямые, выбираем произвольную точку на второй
                Scalar t2 = f / e;
                Point3 q2 = p2 + d2 * t2;
                Scalar t1 = Vector3::dot(q2 - p1, d1) / a;
                return {p1 + d1 * t1, q2};
            }
            Scalar t1 = (b * f - c * e) / denom;
            Scalar t2 = (a * f - b * c) / denom;
            return {p1 + d1 * t1, p2 + d2 * t2};
        };

        Vector3 d1 = vector();
        Vector3 d2 = other.vector();

        // Ищем ближайшие точки на прямых
        auto [p1, p2] = closestPointsBetweenLines(start, d1, other.start, d2);

        // Проверяем, лежат ли они в пределах отрезков и достаточно близко
        Scalar distSq = (p1 - p2).lengthSquared();
        if (distSq <= tolerance * tolerance) {
            bool on1 = contains(p1, tolerance);
            bool on2 = other.contains(p2, tolerance);
            if (on1 && on2) {
                // Возвращаем среднюю точку
                return Point3{
                    (p1.x + p2.x) * Scalar(0.5),
                    (p1.y + p2.y) * Scalar(0.5),
                    (p1.z + p2.z) * Scalar(0.5)
                };
            }
        }
        return std::nullopt;
    }

    // ── Расстояние между двумя отрезками ────────────────────
    [[nodiscard]] static Scalar distanceBetween(const Segment3& a, const Segment3& b) noexcept {
        // Используем ту же технику, что и в intersect, но с обрезанием параметров
        auto closestBetweenLines = [](
            const Point3& p1, const Vector3& d1,
            const Point3& p2, const Vector3& d2) -> std::pair<Point3, Point3>
        {
            Vector3 r = p1 - p2;
            Scalar a = Vector3::dot(d1, d1);
            Scalar e = Vector3::dot(d2, d2);
            Scalar f = Vector3::dot(d2, r);

            if (a < Scalar(1e-20) && e < Scalar(1e-20)) return {p1, p2};
            if (a < Scalar(1e-20)) {
                Scalar t2 = f / e;
                return {p1, p2 + d2 * t2};
            }
            if (e < Scalar(1e-20)) {
                Scalar t1 = -Vector3::dot(d1, r) / a;
                return {p1 + d1 * t1, p2};
            }
            Scalar c = Vector3::dot(d1, r);
            Scalar b = Vector3::dot(d1, d2);
            Scalar denom = a * e - b * b;
            if (std::abs(denom) < Scalar(1e-20)) {
                Scalar t2 = f / e;
                Point3 q2 = p2 + d2 * t2;
                Scalar t1 = Vector3::dot(q2 - p1, d1) / a;
                return {p1 + d1 * t1, q2};
            }
            Scalar t1 = (b * f - c * e) / denom;
            Scalar t2 = (a * f - b * c) / denom;
            return {p1 + d1 * t1, p2 + d2 * t2};
        };

        Vector3 d1 = a.vector();
        Vector3 d2 = b.vector();
        Scalar lenSqA = d1.lengthSquared();
        Scalar lenSqB = d2.lengthSquared();

        // Функция для обрезания точки до отрезка
        auto clampToSegment = [](const Point3& p, const Segment3& seg, const Vector3& dir, Scalar lenSq) {
            if (lenSq < Scalar(1e-20)) return seg.start;
            Scalar t = Vector3::dot(p - seg.start, dir) / lenSq;
            t = std::clamp(t, Scalar(0), Scalar(1));
            return seg.start + dir * t;
        };

        auto [pA, pB] = closestBetweenLines(a.start, d1, b.start, d2);

        Point3 clampedA = clampToSegment(pA, a, d1, lenSqA);
        Point3 clampedB = clampToSegment(pB, b, d2, lenSqB);

        // Пересчитываем ближайшие точки после фиксации одного из концов
        Point3 closestToA = b.closestPoint(clampedA);
        Point3 closestToB = a.closestPoint(clampedB);

        Scalar dist1 = Point3::distance(clampedA, closestToA);
        Scalar dist2 = Point3::distance(closestToB, clampedB);

        return std::min(dist1, dist2);
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Segment3& a, const Segment3& b) noexcept {
        return a.start == b.start && a.end == b.end;
    }
    friend constexpr bool operator!=(const Segment3& a, const Segment3& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir