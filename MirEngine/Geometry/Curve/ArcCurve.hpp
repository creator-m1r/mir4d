// MirEngine/Geometry/Curve/ArcCurve.hpp
// 🌙 Дуга как параметрическая кривая — обёртка над Arc3.
//
// ArcCurve превращает геометрическую дугу (Arc3) в параметрическую кривую,
// которую можно использовать везде, где ожидается ParametricCurve.
// Параметр t = 0.0 соответствует начальной точке дуги, t = 1.0 — конечной.
// Внутри используется линейная интерполяция угла от startAngle до endAngle.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "ParametricCurve.hpp"
#include "../Arc/Arc3.hpp"
#include "Core/Types/Angle.hpp"             // mir::Angle
#include <cmath>

namespace mir {

class ArcCurve : public ParametricCurve {
public:
    explicit ArcCurve(const Arc3& arc) noexcept : m_arc(arc) {}

    [[nodiscard]] Point3 pointAt(Scalar t) const override {
        t = std::clamp(t, Scalar(0), Scalar(1));
        Scalar angle = m_arc.startAngle + t * m_arc.sweptAngle();
        return m_arc.circle.pointAtAngle(Angle::radians(angle));
    }

    [[nodiscard]] Vector3 tangentAt(Scalar t) const override {
        t = std::clamp(t, Scalar(0), Scalar(1));
        Scalar angle = m_arc.startAngle + t * m_arc.sweptAngle();
        // Единичный касательный вектор в плоскости окружности:
        // T = (-sin(θ), cos(θ)) в локальной системе (X,Y)
        Vector3 xAxis = getLocalXAxis();
        Vector3 yAxis = Vector3::cross(m_arc.normal().asVector(), xAxis);
        // Угол θ — угол от оси X в локальной плоскости
        Scalar cosA = std::cos(angle);
        Scalar sinA = std::sin(angle);
        // Касательный вектор: -sinA * xAxis + cosA * yAxis (направление обхода)
        return (yAxis * cosA) - (xAxis * sinA);
    }

    // Длина дуги (не виртуальная, если в базовом классе нет соответствующей virtual)
    [[nodiscard]] Scalar length() const noexcept {
        return m_arc.length();
    }

    [[nodiscard]] const Arc3& arc() const noexcept { return m_arc; }

private:
    Arc3 m_arc;

    // Локальная ось X в плоскости окружности, перпендикулярная нормали.
    [[nodiscard]] Vector3 getLocalXAxis() const noexcept {
        Vector3 candidate = Vector3::cross(m_arc.normal().asVector(), Vector3::unitX());
        if (candidate.lengthSquared() < Scalar(1e-10)) {
            candidate = Vector3::cross(m_arc.normal().asVector(), Vector3::unitY());
        }
        return candidate.normalized();
    }
};

} // namespace mir