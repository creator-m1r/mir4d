// MirEngine/Geometry/Arc/Arc3.hpp
// 🌙 Дуга окружности в трёхмерном пространстве — часть окружности,
//    ограниченная двумя углами.
//
// Дуга (Arc3) — это "кусочек" окружности. В то время как полная окружность
// охватывает все 360°, дуга — только часть, например от 30° до 120°.
// Она задаётся теми же параметрами, что и окружность (центр, нормаль, радиус),
// но дополнительно имеет начальный и конечный углы, которые определяют,
// где дуга начинается и заканчивается.
//
// Дуги широко используются в:
//   • Эскизах (Sketch) — контуры деталей часто состоят из линий и дуг.
//   • Скруглениях (Fillet) — скругление угла = дуга между двумя прямыми.
//   • Инструментах построения — выдавливание (Extrude) по дуге, вращение (Revolve).
//   • Черчении — размерные линии, обозначения углов.
//
// Важные детали реализации:
//   • Углы хранятся в радианах (как и везде в MirEngine).
//   • Порядок: startAngle → endAngle ПРОТИВ часовой стрелки, если смотреть
//     с конца вектора normal (правило правой руки).
//   • Если startAngle > endAngle, дуга проходит через 0° (например, от 350° до 10°).
//   • Дуга может быть "полной окружностью", если углы совпадают (например, 0 и 2π).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Circle/Circle3.hpp"        // mir::Circle3
#include "../Point/Point3.hpp"           // mir::Point3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include "../../Core/Types/Angle.hpp"    // mir::Angle
#include <cmath>
#include <algorithm>

namespace mir {

class Arc3 {
public:
    // ── Компоненты ───────────────────────────────────────────
    Circle3 circle;         // базовая окружность (центр, нормаль, радиус)
    Scalar  startAngle;     // начальный угол в радианах (0 = локальная ось X')
    Scalar  endAngle;       // конечный угол в радианах

    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт дугу по окружности и двум углам.
    constexpr Arc3(const Circle3& circle, Scalar startAngle, Scalar endAngle) noexcept
        : circle(circle)
        , startAngle(startAngle)
        , endAngle(endAngle)
    {}

    // Создаёт дугу по центру, нормали, радиусу и углам.
    constexpr Arc3(const Point3& center, const Direction3& normal, Scalar radius,
                   Scalar startAngle, Scalar endAngle) noexcept
        : circle(Circle3(center, normal, radius))
        , startAngle(startAngle)
        , endAngle(endAngle)
    {}

    // Создаёт дугу по трём точкам (начало, промежуточная, конец).
    // Порядок точек определяет направление дуги и плоскость.
    [[nodiscard]] static Arc3 fromThreePoints(const Point3& start, const Point3& mid, const Point3& end) noexcept {
        // Строим окружность через три точки
        Circle3 circ = Circle3::fromThreePoints(start, mid, end);

        // Вычисляем углы точек относительно центра окружности
        Vector3 xAxis = Vector3::cross(circ.normal.asVector(), Vector3::unitY());
        if (xAxis.lengthSquared() < 1e-10) {
            xAxis = Vector3::cross(circ.normal.asVector(), Vector3::unitX());
        }
        xAxis = xAxis.normalized();
        Vector3 yAxis = Vector3::cross(circ.normal.asVector(), xAxis).normalized();

        auto angleOf = [&](const Point3& p) -> Scalar {
            Vector3 v = p - circ.center;
            Scalar dx = Vector3::dot(v, xAxis);
            Scalar dy = Vector3::dot(v, yAxis);
            Scalar angle = std::atan2(dy, dx);
            if (angle < 0.0) angle += 2.0 * 3.14159265358979323846;
            return angle;
        };

        Scalar a1 = angleOf(start);
        Scalar a2 = angleOf(mid);
        Scalar a3 = angleOf(end);

        // Проверяем порядок обхода: a1 -> a2 -> a3 должны идти против часовой стрелки
        // Если нет — меняем направление
        bool ccw = (a1 <= a2 && a2 <= a3) || (a1 > a2 && a2 > a3) ||
                   (a2 <= a3 && a3 <= a1) || (a3 <= a1 && a1 <= a2);
        // Упрощённо: проверяем знак векторного произведения (start->mid) × (mid->end)
        Vector3 d1 = mid - start;
        Vector3 d2 = end - mid;
        Scalar crossSign = Vector3::dot(Vector3::cross(d1, d2), circ.normal.asVector());
        if (crossSign < 0.0) {
            // Меняем направление
            return Arc3(circ, a3, a1);
        }
        return Arc3(circ, a1, a3);
    }

    // ── Геометрические свойства ─────────────────────────────

    // Длина дуги.
    [[nodiscard]] Scalar length() const noexcept {
        Scalar sweep = sweptAngle();
        return circle.radius * sweep;
    }

    // Угол, заметаемый дугой (в радианах).
    [[nodiscard]] Scalar sweptAngle() const noexcept {
        Scalar diff = endAngle - startAngle;
        if (diff < 0.0) {
            diff += 2.0 * 3.14159265358979323846;
        }
        return diff;
    }

    // Центр базовой окружности.
    [[nodiscard]] constexpr Point3 center() const noexcept {
        return circle.center;
    }

    // Радиус.
    [[nodiscard]] constexpr Scalar radius() const noexcept {
        return circle.radius;
    }

    // Нормаль (направление "вверх" от плоскости дуги).
    [[nodiscard]] constexpr Direction3 normal() const noexcept {
        return circle.normal;
    }

    // ── Точки на дуге ────────────────────────────────────────

    // Начальная точка дуги.
    [[nodiscard]] Point3 startPoint() const noexcept {
        return circle.pointAtAngle(Angle::radians(startAngle));
    }

    // Конечная точка дуги.
    [[nodiscard]] Point3 endPoint() const noexcept {
        return circle.pointAtAngle(Angle::radians(endAngle));
    }

    // Точка на дуге по параметру t (0 = startPoint, 1 = endPoint).
    [[nodiscard]] Point3 pointAt(Scalar t) const noexcept {
        Scalar sweep = sweptAngle();
        Scalar angle = startAngle + t * sweep;
        return circle.pointAtAngle(Angle::radians(angle));
    }

    // Точка на дуге по заданному углу (в радианах).
    [[nodiscard]] Point3 pointAtAngle(const Angle& angle) const noexcept {
        return circle.pointAtAngle(angle);
    }

    // ── Проверка принадлежности точки дуге ──────────────────
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-10) const noexcept {
        // Сначала проверяем, лежит ли точка на окружности
        if (!circle.contains(point, tolerance)) return false;

        // Вычисляем угол точки и проверяем, лежит ли он в диапазоне [startAngle, endAngle]
        Scalar angle = angleOfPoint(point);
        return isAngleInRange(angle, tolerance);
    }

    // ── Ближайшая точка на дуге к заданной точке ─────────────
    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept {
        // Находим ближайшую точку на полной окружности
        Point3 onCircle = circle.closestPoint(point);

        // Проверяем, лежит ли она в пределах дуги
        Scalar angle = angleOfPoint(onCircle);
        if (isAngleInRange(angle)) {
            return onCircle;
        }

        // Если нет — ближайшая точка это один из концов дуги
        Point3 s = startPoint();
        Point3 e = endPoint();
        Scalar d1 = Point3::distance(point, s);
        Scalar d2 = Point3::distance(point, e);
        return (d1 <= d2) ? s : e;
    }

    // ── Разбиение дуги на сегменты ───────────────────────────
    // Возвращает n равноотстоящих точек вдоль дуги (включая начало и конец).
    [[nodiscard]] std::vector<Point3> tessellate(int numSegments) const noexcept {
        std::vector<Point3> points;
        points.reserve(numSegments + 1);
        for (int i = 0; i <= numSegments; ++i) {
            Scalar t = Scalar(i) / Scalar(numSegments);
            points.push_back(pointAt(t));
        }
        return points;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Arc3& a, const Arc3& b) noexcept {
        return a.circle == b.circle && a.startAngle == b.startAngle && a.endAngle == b.endAngle;
    }
    friend constexpr bool operator!=(const Arc3& a, const Arc3& b) noexcept {
        return !(a == b);
    }

private:
    // Вычисляет угол точки относительно центра дуги (в радианах, [0, 2π)).
    [[nodiscard]] Scalar angleOfPoint(const Point3& point) const noexcept {
        // Используем локальный базис (тот же, что в Circle3)
        Vector3 xAxis = getLocalXAxis();
        Vector3 yAxis = Vector3::cross(circle.normal.asVector(), xAxis);
        Vector3 v = point - circle.center;
        Scalar dx = Vector3::dot(v, xAxis);
        Scalar dy = Vector3::dot(v, yAxis);
        Scalar angle = std::atan2(dy, dx);
        if (angle < 0.0) angle += 2.0 * 3.14159265358979323846;
        return angle;
    }

    // Проверяет, лежит ли угол в диапазоне [startAngle, endAngle].
    [[nodiscard]] bool isAngleInRange(Scalar angle, Scalar tolerance = 1e-10) const noexcept {
        Scalar start = startAngle;
        Scalar end = endAngle;
        if (end < start) {
            end += 2.0 * 3.14159265358979323846;
        }
        // Нормализуем угол относительно start
        Scalar normAngle = angle;
        while (normAngle < start - tolerance) normAngle += 2.0 * 3.14159265358979323846;
        while (normAngle > end + tolerance) normAngle -= 2.0 * 3.14159265358979323846;
        return normAngle >= start - tolerance && normAngle <= end + tolerance;
    }

    // Возвращает локальную ось X в плоскости окружности.
    [[nodiscard]] Vector3 getLocalXAxis() const noexcept {
        Vector3 candidate = Vector3::cross(circle.normal.asVector(), Vector3::unitX());
        if (candidate.lengthSquared() < 1e-10) {
            candidate = Vector3::cross(circle.normal.asVector(), Vector3::unitY());
        }
        return candidate.normalized();
    }
};

} // namespace mir