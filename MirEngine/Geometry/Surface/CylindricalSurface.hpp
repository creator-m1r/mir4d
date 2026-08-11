// MirEngine/Geometry/Surface/CylindricalSurface.hpp
// 🥫 Цилиндрическая поверхность — бесконечный цилиндр, заданный осью и радиусом.
//
// CylindricalSurface — это реализация Surface для цилиндра. Цилиндр
// описывается осью (прямая линия) и радиусом. Поверхность цилиндра
// состоит из всех точек, находящихся на расстоянии radius от оси.
//
// Параметризация:
//   • u (0..1) — угол вокруг оси (0 = локальная ось X', 1 = 2π).
//   • v (-∞..+∞) — расстояние вдоль оси от начальной точки.
//
// Такая параметризация удобна для текстурирования, разбиения на сетку
// и других операций, где нужна UV-параметризация.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Surface.hpp"                   // Базовый класс Surface
#include "../Line/Line3.hpp"            // Ось цилиндра
#include "../Direction/Direction3.hpp"   // Направление оси
#include "../../Math/Vector/Vector3.hpp" // Vector3
#include "../../Core/Types/Scalar.hpp"   // Scalar
#include <cmath>
#include <optional>
#include <tuple>

namespace mir {

class CylindricalSurface : public Surface {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт цилиндрическую поверхность по оси и радиусу.
    // Ось не обязана проходить через начало координат.
    CylindricalSurface(const Line3& axis, Scalar radius) noexcept
        : m_axis(axis)
        , m_radius(radius > Scalar(0) ? radius : Scalar(1))
    {
        // Строим ортонормированный базис в плоскости, перпендикулярной оси.
        m_axisDir = Direction3::fromVector(m_axis.direction);
        // Ось U — любой вектор, ортогональный оси.
        Vector3 candidateU = Vector3::cross(m_axisDir.asVector(), Vector3::unitX());
        if (candidateU.lengthSquared() < Scalar(1e-10)) {
            candidateU = Vector3::cross(m_axisDir.asVector(), Vector3::unitY());
        }
        m_localU = candidateU.normalized();
        // Ось V — ортогональна и к оси, и к localU (правило правой руки).
        m_localV = Vector3::cross(m_axisDir.asVector(), m_localU).normalized();
    }

    // ── Доступ к данным ─────────────────────────────────────
    [[nodiscard]] const Line3& axis()   const noexcept { return m_axis; }
    [[nodiscard]] Scalar      radius() const noexcept { return m_radius; }

    // ── Реализация Surface ──────────────────────────────────

    // Точка на цилиндре при параметрах (u, v).
    // u ∈ [0, 1] отображается в угол [0, 2π].
    // v ∈ (-∞, +∞) — смещение вдоль оси.
    [[nodiscard]] Point3 pointAt(Scalar u, Scalar v) const override {
        Scalar angle = u * Scalar(2) * Scalar(3.14159265358979323846);
        Scalar cosA = std::cos(angle);
        Scalar sinA = std::sin(angle);
        // Точка на окружности в плоскости, перпендикулярной оси.
        Vector3 radial = (m_localU * cosA) + (m_localV * sinA);
        return m_axis.origin + (radial * m_radius) + (m_axisDir.asVector() * v);
    }

    // Нормаль в точке (направлена наружу от оси).
    [[nodiscard]] Direction3 normalAt(Scalar u, Scalar v) const override {
        Scalar angle = u * Scalar(2) * Scalar(3.14159265358979323846);
        Scalar cosA = std::cos(angle);
        Scalar sinA = std::sin(angle);
        Vector3 radial = (m_localU * cosA) + (m_localV * sinA);
        return Direction3::fromVector(radial);
    }

    // Производная по u (касательная вдоль окружности).
    [[nodiscard]] Vector3 derivativeU(Scalar u, Scalar /*v*/) const override {
        Scalar angle = u * Scalar(2) * Scalar(3.14159265358979323846);
        Scalar cosA = std::cos(angle);
        Scalar sinA = std::sin(angle);
        // Производная радиального вектора по углу, умноженная на du/dt = 2π.
        Vector3 dRadial = (m_localV * cosA) - (m_localU * sinA);
        return dRadial * (Scalar(2) * Scalar(3.14159265358979323846) * m_radius);
    }

    // Производная по v (вдоль оси).
    [[nodiscard]] Vector3 derivativeV(Scalar /*u*/, Scalar /*v*/) const override {
        return m_axisDir.asVector();
    }

    // Ближайшие параметры (u, v) для заданной точки.
    [[nodiscard]] std::pair<Scalar, Scalar> closestParameters(
        const Point3& point, int /*samplesU*/ = 0, int /*samplesV*/ = 0) const noexcept override {
        // Проецируем точку на ось цилиндра — это даёт v.
        Vector3 toPoint = point - m_axis.origin;
        Scalar v = Vector3::dot(toPoint, m_axisDir.asVector());

        // Проекция точки на плоскость, перпендикулярную оси.
        Point3 projectedOnAxis = m_axis.origin + (m_axisDir.asVector() * v);
        Vector3 radialVec = point - projectedOnAxis;
        Scalar radialDist = radialVec.length();

        // Если точка на оси, угол не определён — возвращаем u=0.
        Scalar u = Scalar(0);
        if (radialDist > Scalar(1e-10)) {
            Vector3 radialDir = radialVec / radialDist;
            Scalar dotU = Vector3::dot(radialDir, m_localU);
            Scalar dotV = Vector3::dot(radialDir, m_localV);
            u = std::atan2(dotV, dotU) / (Scalar(2) * Scalar(3.14159265358979323846));
            if (u < Scalar(0)) u += Scalar(1);
        }
        return {u, v};
    }

    // Проверка принадлежности: расстояние до оси должно быть ≈ radius.
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept override {
        auto [u, v] = closestParameters(point);
        return std::abs(Point3::distance(point, pointAt(u, v))) <= tolerance;
    }

    // Пересечение с лучом (аналитическое решение через квадратное уравнение).
    [[nodiscard]] std::optional<std::tuple<Point3, Scalar, Scalar>> intersect(
        const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const noexcept override {
        // Алгоритм: решаем квадратное уравнение |(ray.origin + t*ray.dir) × axis.dir|² = r²
        Vector3 ro = ray.origin - m_axis.origin;
        Vector3 rd = ray.direction;
        Vector3 ad = m_axis.direction.normalized();

        Scalar a = Vector3::dot(rd, rd) - std::pow(Vector3::dot(rd, ad), Scalar(2));
        Scalar b = Scalar(2) * (Vector3::dot(rd, ro) - Vector3::dot(rd, ad) * Vector3::dot(ro, ad));
        Scalar c = Vector3::dot(ro, ro) - std::pow(Vector3::dot(ro, ad), Scalar(2)) - m_radius * m_radius;

        Scalar discriminant = b * b - Scalar(4) * a * c;
        if (discriminant < Scalar(0)) return std::nullopt;

        Scalar t1 = (-b - std::sqrt(discriminant)) / (Scalar(2) * a);
        Scalar t2 = (-b + std::sqrt(discriminant)) / (Scalar(2) * a);

        // Берём ближайшее положительное t.
        Scalar t = (t1 > tolerance) ? t1 : (t2 > tolerance) ? t2 : -Scalar(1);
        if (t < tolerance) return std::nullopt;

        Point3 hitPoint = ray.pointAt(t);
        auto [u, v] = closestParameters(hitPoint);
        return std::make_tuple(hitPoint, u, v);
    }

private:
    Line3   m_axis;     // ось цилиндра (точка + направление)
    Scalar  m_radius;   // радиус цилиндра
    Direction3 m_axisDir;  // единичное направление оси
    Vector3 m_localU;      // локальная ось U (ортогональна оси)
    Vector3 m_localV;      // локальная ось V (ортогональна оси и localU)
};

} // namespace mir