// MirEngine/Geometry/Curve/LineCurve.hpp
// ➖ Отрезок как параметрическая кривая — самая простая реализация ParametricCurve.
//
// LineCurve — это адаптер, который превращает обычный отрезок (Segment3)
// в параметрическую кривую. Это нужно, чтобы можно было работать с отрезком
// так же, как с любой другой кривой: получать точку по параметру t,
// вычислять длину, искать ближайшую точку, разбивать на сегменты.
//
// Параметр t пробегает от 0.0 (начало отрезка) до 1.0 (конец отрезка).
// Это стандартное поведение для всех параметрических кривых в MirEngine.
//
// LineCurve используется для:
//   • Представления рёбер в эскизах (Sketch).
//   • Построения ломаных линий (полилиний).
//   • Выдавливания (Extrude) вдоль прямого пути.
//   • Унификации работы с геометрией: любой отрезок можно передать туда,
//     где ожидается ParametricCurve.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "ParametricCurve.hpp"       // Базовый класс ParametricCurve
#include "../Segment/Segment3.hpp"   // mir::Segment3
#include "../Point/Point3.hpp"       // mir::Point3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <algorithm>                 // для std::clamp

namespace mir {

class LineCurve : public ParametricCurve {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт кривую-отрезок из Segment3.
    explicit LineCurve(const Segment3& segment) noexcept
        : m_segment(segment)
    {}

    // Создаёт кривую-отрезок из двух точек (начало и конец).
    LineCurve(const Point3& start, const Point3& end) noexcept
        : m_segment(Segment3(start, end))
    {}

    // ── Реализация ParametricCurve ──────────────────────────

    // Точка на отрезке при параметре t.
    [[nodiscard]] Point3 pointAt(Scalar t) const override {
        t = std::clamp(t, Scalar(0), Scalar(1));
        return m_segment.pointAt(t);
    }

    // Касательный вектор (направление отрезка) — константа по всей длине.
    [[nodiscard]] Vector3 tangentAt(Scalar /*t*/) const override {
        return m_segment.vector();   // вектор от start к end
    }

    // Длина отрезка (аналитически, без численного интегрирования).
    [[nodiscard]] Scalar length(int /*numSamples*/ = 0) const noexcept {
        return m_segment.length();
    }

    // Ближайший параметр t (аналитически, без перебора).
    [[nodiscard]] Scalar closestParameter(const Point3& point, int /*numSamples*/ = 0) const noexcept override {
        Vector3 dir = m_segment.vector();
        Scalar lenSq = dir.lengthSquared();
        if (lenSq < Scalar(1e-20)) {
            return Scalar(0);   // отрезок вырожден в точку
        }
        Scalar t = Vector3::dot(point - m_segment.start, dir) / lenSq;
        return std::clamp(t, Scalar(0), Scalar(1));
    }

    // ── Доступ к исходному отрезку ─────────────────────────
    [[nodiscard]] const Segment3& segment() const noexcept {
        return m_segment;
    }

private:
    Segment3 m_segment;   // хранимый отрезок
};

} // namespace mir