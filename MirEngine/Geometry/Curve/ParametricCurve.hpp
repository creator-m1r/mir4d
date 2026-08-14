// MirEngine/Geometry/Curve/ParametricCurve.hpp
// 📈 Базовый класс для всех параметрических кривых в MirEngine.
//
// Параметрическая кривая — это математический объект, который описывает
// плавную линию в пространстве с помощью одного параметра t.
// Когда t пробегает от 0 до 1, точка на кривой движется от начала к концу.
// Это основной способ представления кривых в CAD: линии, дуги, сплайны Безье,
// NURBS — все они наследуются от этого класса.
//
// Как это работает:
//   • Метод pointAt(t) возвращает точку на кривой при заданном t.
//   • Метод tangentAt(t) возвращает направление кривой в этой точке.
//   • Метод length() вычисляет длину кривой (численным интегрированием).
//   • Метод closestPoint(p) находит ближайшую точку на кривой к заданной точке.
//
// Это абстрактный класс — нельзя создать просто ParametricCurve,
// нужно создать конкретную кривую (LineCurve, BezierCurve, NurbsCurve...).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <vector>
#include <cmath>

namespace mir {

class ParametricCurve {
public:
    virtual ~ParametricCurve() = default;

    // ── Основные методы (должны быть реализованы в наследниках) ──

    // Точка на кривой при параметре t (0.0 — начало, 1.0 — конец).
    [[nodiscard]] virtual Point3 pointAt(Scalar t) const = 0;

    // Касательный вектор в точке t (направление, не нормализован).
    [[nodiscard]] virtual Vector3 tangentAt(Scalar t) const {
        // Численное дифференцирование (по умолчанию).
        // Наследники могут переопределить для точного аналитического решения.
        Scalar h = 1e-6;
        Scalar t0 = (t > 0.5) ? t - h : t;
        Scalar t1 = t0 + h;
        return (pointAt(t1) - pointAt(t0)) / h;
    }

    // Направление (единичный вектор) в точке t.
    [[nodiscard]] Vector3 directionAt(Scalar t) const {
        Vector3 tang = tangentAt(t);
        Scalar len = tang.length();
        return (len > 1e-20) ? tang / len : Vector3::unitX();
    }

    // ── Длина кривой (численное интегрирование) ─────────────

    // Полная длина кривой.
    [[nodiscard]] Scalar length(int numSamples = 100) const noexcept {
        Scalar total = 0.0;
        Point3 prev = pointAt(0.0);
        for (int i = 1; i <= numSamples; ++i) {
            Scalar t = Scalar(i) / Scalar(numSamples);
            Point3 curr = pointAt(t);
            total += Point3::distance(prev, curr);
            prev = curr;
        }
        return total;
    }

    // Длина от tStart до tEnd.
    [[nodiscard]] Scalar lengthBetween(Scalar tStart, Scalar tEnd, int numSamples = 50) const noexcept {
        tStart = std::clamp(tStart, Scalar(0), Scalar(1));
        tEnd   = std::clamp(tEnd,   Scalar(0), Scalar(1));
        Scalar total = 0.0;
        Point3 prev = pointAt(tStart);
        for (int i = 1; i <= numSamples; ++i) {
            Scalar t = tStart + (tEnd - tStart) * Scalar(i) / Scalar(numSamples);
            Point3 curr = pointAt(t);
            total += Point3::distance(prev, curr);
            prev = curr;
        }
        return total;
    }

    // ── Ближайшая точка на кривой к заданной точке ─────────

    // Возвращает параметр t, соответствующий ближайшей точке на кривой.
    [[nodiscard]] virtual Scalar closestParameter(const Point3& point, int numSamples = 100) const noexcept {
        Scalar bestT = 0.0;
        Scalar bestDist = std::numeric_limits<Scalar>::max();

        // Грубый поиск по сетке
        for (int i = 0; i <= numSamples; ++i) {
            Scalar t = Scalar(i) / Scalar(numSamples);
            Scalar d = Point3::distanceSquared(point, pointAt(t));
            if (d < bestDist) {
                bestDist = d;
                bestT = t;
            }
        }

        // Уточнение методом золотого сечения
        bestT = refineParameter(point, bestT, 1.0 / Scalar(numSamples));
        return bestT;
    }

    // Ближайшая точка на кривой.
    [[nodiscard]] Point3 closestPoint(const Point3& point, int numSamples = 100) const noexcept {
        Scalar t = closestParameter(point, numSamples);
        return pointAt(t);
    }

    // ── Разбиение кривой на отрезки ─────────────────────────

    // Возвращает n точек, равномерно распределённых по параметру t.
    [[nodiscard]] std::vector<Point3> tessellateByParameter(int numSegments) const noexcept {
        std::vector<Point3> points;
        points.reserve(numSegments + 1);
        for (int i = 0; i <= numSegments; ++i) {
            Scalar t = Scalar(i) / Scalar(numSegments);
            points.push_back(pointAt(t));
        }
        return points;
    }

    // Возвращает n точек, равномерно распределённых по длине кривой.
    [[nodiscard]] std::vector<Point3> tessellateByLength(int numSegments) const noexcept {
        Scalar totalLen = length();
        if (totalLen < 1e-20) {
            return {pointAt(0.0)};
        }

        std::vector<Point3> points;
        points.reserve(numSegments + 1);
        points.push_back(pointAt(0.0));

        Scalar stepLen = totalLen / Scalar(numSegments);
        Scalar accumulated = 0.0;
        Point3 prev = pointAt(0.0);
        int sampleIdx = 0;

        for (int i = 1; i < numSegments; ++i) {
            Scalar target = stepLen * Scalar(i);
            while (accumulated < target) {
                sampleIdx++;
                Scalar t = Scalar(sampleIdx) / Scalar(200); // 200 сэмплов
                Point3 curr = pointAt(t);
                accumulated += Point3::distance(prev, curr);
                prev = curr;
                if (t >= 1.0) break;
            }
            points.push_back(prev);
        }
        points.push_back(pointAt(1.0));
        return points;
    }

protected:
    // Уточнение параметра t методом золотого сечения.
    [[nodiscard]] Scalar refineParameter(const Point3& point, Scalar bestT, Scalar initialStep) const noexcept {
        Scalar a = std::max(Scalar(0), bestT - initialStep);
        Scalar b = std::min(Scalar(1), bestT + initialStep);

        constexpr Scalar GOLDEN_RATIO = 0.6180339887498949;
        Scalar x1 = b - GOLDEN_RATIO * (b - a);
        Scalar x2 = a + GOLDEN_RATIO * (b - a);

        for (int i = 0; i < 20; ++i) {
            Scalar d1 = Point3::distanceSquared(point, pointAt(x1));
            Scalar d2 = Point3::distanceSquared(point, pointAt(x2));

            if (d1 < d2) {
                b = x2;
                x2 = x1;
                x1 = b - GOLDEN_RATIO * (b - a);
            } else {
                a = x1;
                x1 = x2;
                x2 = a + GOLDEN_RATIO * (b - a);
            }
        }
        return (a + b) * 0.5;
    }
};

} // namespace mir