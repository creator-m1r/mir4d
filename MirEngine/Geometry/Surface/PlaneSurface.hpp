// MirEngine/Geometry/Surface/PlaneSurface.hpp
// 📐 Плоская поверхность — бесконечная плоскость как параметрическая поверхность.
//
// PlaneSurface — это реализация Surface для плоскости. В отличие от
// геометрического класса Plane (который только хранит точку и нормаль),
// PlaneSurface добавляет параметризацию: два ортогональных вектора (ось U и ось V),
// которые задают "координатную сетку" на плоскости. Это позволяет получать
// точку по параметрам (u, v), вычислять производные и разбивать плоскость
// на сетку для отрисовки.
//
// Параметризация:
//   • pointAt(u, v) = origin + u * axisU + v * axisV
//   • u, v ∈ (-∞, +∞) — плоскость бесконечна.
//   • Для ограниченной плоскости (прямоугольника) нужно задать диапазоны u, v.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Surface.hpp"                   // Базовый класс Surface
#include "../Plane/Plane.hpp"            // mir::Plane
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar

namespace mir {

class PlaneSurface : public Surface {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт плоскость из геометрического объекта Plane.
    // Автоматически вычисляет оси U и V (ортонормированный базис).
    explicit PlaneSurface(const Plane& plane) noexcept
        : m_plane(plane)
    {
        // Строим ортонормированный базис в плоскости.
        // axisU = нормализованный вектор, ортогональный нормали и unitX (или unitY).
        Vector3 candidateU = Vector3::cross(plane.normal.asVector(), Vector3::unitX());
        if (candidateU.lengthSquared() < Scalar(1e-10)) {
            candidateU = Vector3::cross(plane.normal.asVector(), Vector3::unitY());
        }
        m_axisU = candidateU.normalized();
        m_axisV = Vector3::cross(plane.normal.asVector(), m_axisU).normalized();
    }

    // Создаёт плоскость по точке и нормали.
    PlaneSurface(const Point3& origin, const Direction3& normal) noexcept
        : PlaneSurface(Plane(origin, normal))
    {}

    // ── Доступ к геометрическому объекту ────────────────────
    [[nodiscard]] const Plane& plane() const noexcept { return m_plane; }
    [[nodiscard]] const Vector3& axisU() const noexcept { return m_axisU; }
    [[nodiscard]] const Vector3& axisV() const noexcept { return m_axisV; }

    // ── Реализация Surface ──────────────────────────────────

    // Точка на плоскости по параметрам (u, v).
    [[nodiscard]] Point3 pointAt(Scalar u, Scalar v) const override {
        return m_plane.origin + (m_axisU * u) + (m_axisV * v);
    }

    // Направление нормали к плоскости.
    // Direction3 уже гарантирует единичную длину, поэтому просто возвращаем его.
    [[nodiscard]] Direction3 normalAt(Scalar /*u*/, Scalar /*v*/) const override {
        return m_plane.normal;
    }

    // Частная производная по u — это ось U.
    [[nodiscard]] Vector3 derivativeU(Scalar /*u*/, Scalar /*v*/) const override {
        return m_axisU;
    }

    // Частная производная по v — это ось V.
    [[nodiscard]] Vector3 derivativeV(Scalar /*u*/, Scalar /*v*/) const override {
        return m_axisV;
    }

    // Ближайшая точка — проекция на плоскость (аналитически).
    [[nodiscard]] std::pair<Scalar, Scalar> closestParameters(
        const Point3& point, int /*samplesU*/ = 0, int /*samplesV*/ = 0) const noexcept override {
        Vector3 v = point - m_plane.origin;
        Scalar u = Vector3::dot(v, m_axisU);
        Scalar vv = Vector3::dot(v, m_axisV);
        return {u, vv};
    }

    // Проверка принадлежности — просто расстояние до плоскости.
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept override {
        return m_plane.contains(point, tolerance);
    }

    // Пересечение с лучом (аналитически).
    [[nodiscard]] std::optional<std::tuple<Point3, Scalar, Scalar>> intersect(
        const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const noexcept override {
        auto optPoint = m_plane.intersect(ray);
        if (!optPoint) return std::nullopt;
        auto [u, v] = closestParameters(*optPoint);
        return std::make_tuple(*optPoint, u, v);
    }

private:
    Plane   m_plane;   // геометрическая плоскость (origin + normal)
    Vector3 m_axisU;   // ось U в плоскости (ортонормирована)
    Vector3 m_axisV;   // ось V в плоскости (ортонормирована, ⊥ m_axisU и normal)
};

} // namespace mir