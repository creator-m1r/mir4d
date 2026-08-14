// MirEngine/Geometry/Curve/NurbsCurve.hpp
// 🎯 NURBS-кривая — самый мощный и универсальный тип кривых в геометрии.
//
// NURBS расшифровывается как Non-Uniform Rational B-Spline. Это математический
// "швейцарский нож" для кривых: с помощью NURBS можно описать ЛЮБУЮ гладкую
// форму — от простого отрезка до сложнейших обводов автомобиля или фюзеляжа
// самолёта. Все современные CAD-системы (SolidWorks, CATIA, Rhino) используют
// NURBS как основной способ представления кривых и поверхностей.
//
// Почему NURBS такой крутой?
//   1. Может ТОЧНО представить окружности, эллипсы, параболы (через веса).
//   2. Позволяет локально редактировать форму: подвинул одну точку —
//      изменился только ближайший участок кривой.
//   3. Можно добавить больше деталей в нужном месте через вставку узлов
//      (insertKnot), не меняя форму кривой.
//   4. Степень гладкости можно контролировать: от ломаной до "идеально гладкой".
//
// Как это работает (упрощённо):
//   • Узловой вектор (knots) разбивает параметр t на участки.
//   • Базисные функции определяют "влияние" каждой контрольной точки в точке t.
//   • Контрольные точки с весами задают форму: больший вес = сильнее "притягивает".
//   • Точка на кривой = взвешенная сумма контрольных точек.
//
// Основные операции:
//   • pointAt(t)        — точка на кривой.
//   • insertKnot(u)     — вставить узел (больше контроля).
//   • refineKnots(n)    — равномерно добавить узлы.
//   • elevateDegree()   — повысить степень (больше гладкости).
//   • decomposeToBezier() — разбить на куски Безье (для рендеринга).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "ParametricCurve.hpp"
#include "../Point/Point3.hpp"
#include "../../Core/Types/Scalar.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mir {

class NurbsCurve : public ParametricCurve {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт NURBS-кривую.
    //   controlPoints — опорные точки (минимум degree+1 штук).
    //   weights       — веса точек (положительные, обычно 1.0 = нет притяжения).
    //   knots         — узловой вектор (размер = controlPoints.size() + degree + 1).
    //   degree        — степень кривой (1=линейная, 2=квадратичная, 3=кубическая).
    NurbsCurve(std::vector<Point3> controlPoints,
               std::vector<Scalar> weights,
               std::vector<Scalar> knots,
               int degree)
        : m_controlPoints(std::move(controlPoints))
        , m_weights(std::move(weights))
        , m_knots(std::move(knots))
        , m_degree(degree)
    {
        validate();
    }

    // ── Доступ к данным ─────────────────────────────────────

    [[nodiscard]] const std::vector<Point3>& controlPoints() const noexcept { return m_controlPoints; }
    [[nodiscard]] const std::vector<Scalar>& weights()       const noexcept { return m_weights; }
    [[nodiscard]] const std::vector<Scalar>& knots()         const noexcept { return m_knots; }
    [[nodiscard]] int degree() const noexcept { return m_degree; }
    [[nodiscard]] int order()  const noexcept { return m_degree + 1; }

    // ── Точка на кривой ─────────────────────────────────────
    // Использует алгоритм Кокса-де Бура (численно устойчивый).
    [[nodiscard]] Point3 pointAt(Scalar t) const override {
        t = std::clamp(t, knots().front(), knots().back() - Scalar(1e-10));
        std::vector<Scalar> N = basisFunctions(t);
        return weightedSum(N);
    }

    // ── Касательный вектор ──────────────────────────────────
    [[nodiscard]] Vector3 tangentAt(Scalar t) const override {
        t = std::clamp(t, knots().front(), knots().back() - Scalar(1e-10));
        Scalar h = Scalar(1e-6);
        Point3 p0 = pointAt(t - h);
        Point3 p1 = pointAt(t + h);
        return (p1 - p0) / (Scalar(2) * h);
    }

    // ── Базисные функции (алгоритм Кокса-де Бура) ───────────
    [[nodiscard]] std::vector<Scalar> basisFunctions(Scalar t) const noexcept {
        int n = static_cast<int>(m_controlPoints.size());
        std::vector<Scalar> N(n, Scalar(0));

        int span = findSpan(t);
        if (span < 0) return N;

        N[span] = Scalar(1);

        for (int d = 1; d <= m_degree; ++d) {
            Scalar left = t - m_knots[span - d + 1];
            Scalar denomLeft = m_knots[span + 1] - m_knots[span - d + 1];
            if (denomLeft > Scalar(0)) {
                N[span - d] = left / denomLeft * N[span - d + 1];
            }

            for (int i = span - d + 1; i <= span - 1; ++i) {
                Scalar left2 = t - m_knots[i];
                Scalar denomLeft2 = m_knots[i + d] - m_knots[i];
                Scalar right2 = m_knots[i + d + 1] - t;
                Scalar denomRight2 = m_knots[i + d + 1] - m_knots[i + 1];

                Scalar term = Scalar(0);
                if (denomLeft2 > Scalar(0)) term += left2 / denomLeft2 * N[i];
                if (denomRight2 > Scalar(0)) term += right2 / denomRight2 * N[i + 1];
                N[i] = term;
            }

            Scalar right = m_knots[span + d + 1] - t;
            Scalar denomRight = m_knots[span + d + 1] - m_knots[span + 1];
            if (denomRight > Scalar(0)) {
                N[span] = right / denomRight * N[span];
            }
        }

        return N;
    }

    // ── Производные базисных функций (численно) ─────────────
    [[nodiscard]] std::vector<Scalar> basisFunctionDerivatives(Scalar t) const noexcept {
        Scalar h = Scalar(1e-6);
        std::vector<Scalar> N1 = basisFunctions(t + h);
        std::vector<Scalar> N0 = basisFunctions(t - h);
        for (size_t i = 0; i < N1.size(); ++i) {
            N1[i] = (N1[i] - N0[i]) / (Scalar(2) * h);
        }
        return N1;
    }

    // ── Вставка узла ────────────────────────────────────────
    void insertKnot(Scalar u, int times = 1) noexcept {
        for (int r = 0; r < times; ++r) {
            int span = findSpan(u);
            int n = static_cast<int>(m_controlPoints.size());

            std::vector<Point3> newPoints(n + 1);
            std::vector<Scalar> newWeights(n + 1);
            std::vector<Scalar> newKnots(m_knots.size() + 1);

            for (int i = 0; i <= span - m_degree; ++i) {
                newPoints[i] = m_controlPoints[i];
                newWeights[i] = m_weights[i];
            }
            for (int i = span; i < n; ++i) {
                newPoints[i + 1] = m_controlPoints[i];
                newWeights[i + 1] = m_weights[i];
            }

            for (int i = span - m_degree + 1; i <= span; ++i) {
                Scalar alpha = (u - m_knots[i]) / (m_knots[i + m_degree] - m_knots[i]);
                alpha = std::clamp(alpha, Scalar(0), Scalar(1));
                newWeights[i] = (Scalar(1) - alpha) * m_weights[i - 1] + alpha * m_weights[i];
                newPoints[i] = Point3::lerp(m_controlPoints[i - 1], m_controlPoints[i], alpha);
            }

            for (int i = 0; i <= span; ++i) newKnots[i] = m_knots[i];
            newKnots[span + 1] = u;
            for (int i = span + 1; i < static_cast<int>(m_knots.size()); ++i) {
                newKnots[i + 1] = m_knots[i];
            }

            m_controlPoints = std::move(newPoints);
            m_weights = std::move(newWeights);
            m_knots = std::move(newKnots);
        }
    }

    // ── Равномерное добавление узлов ────────────────────────
    void refineKnots(int numNewKnots = 5) noexcept {
        Scalar uMin = m_knots.front();
        Scalar uMax = m_knots.back();
        Scalar step = (uMax - uMin) / Scalar(numNewKnots + 1);
        for (int i = 1; i <= numNewKnots; ++i) {
            insertKnot(uMin + step * Scalar(i));
        }
    }

    // ── Повышение степени ───────────────────────────────────
    void elevateDegree() noexcept {
        int n = static_cast<int>(m_controlPoints.size());
        int newDegree = m_degree + 1;
        std::vector<Point3> newPoints;
        std::vector<Scalar> newWeights;
        std::vector<Scalar> newKnots;

        newPoints.push_back(m_controlPoints[0]);
        newWeights.push_back(m_weights[0]);

        for (int i = 1; i < n; ++i) {
            Scalar alpha = Scalar(i) / Scalar(newDegree);
            newPoints.push_back(Point3::lerp(m_controlPoints[i], m_controlPoints[i - 1], alpha));
            newWeights.push_back((Scalar(1) - alpha) * m_weights[i] + alpha * m_weights[i - 1]);
        }

        newPoints.push_back(m_controlPoints.back());
        newWeights.push_back(m_weights.back());

        newKnots.push_back(m_knots.front());
        for (int i = 1; i < static_cast<int>(m_knots.size()) - 1; ++i) {
            newKnots.push_back(m_knots[i]);
            newKnots.push_back(m_knots[i]);
        }
        newKnots.push_back(m_knots.back());

        m_controlPoints = std::move(newPoints);
        m_weights = std::move(newWeights);
        m_knots = std::move(newKnots);
        m_degree = newDegree;
    }

private:
    std::vector<Point3> m_controlPoints;
    std::vector<Scalar> m_weights;
    std::vector<Scalar> m_knots;
    int m_degree = 3;

    void validate() {
        if (m_controlPoints.size() < static_cast<size_t>(m_degree + 1)) {
            throw std::invalid_argument("NurbsCurve: недостаточно контрольных точек (нужно минимум degree+1)");
        }
        if (m_weights.size() != m_controlPoints.size()) {
            throw std::invalid_argument("NurbsCurve: количество весов должно совпадать с количеством контрольных точек");
        }
        if (m_knots.size() != m_controlPoints.size() + m_degree + 1) {
            throw std::invalid_argument("NurbsCurve: неверный размер узлового вектора (должен быть points + degree + 1)");
        }
        for (size_t i = 1; i < m_knots.size(); ++i) {
            if (m_knots[i] < m_knots[i - 1]) {
                throw std::invalid_argument("NurbsCurve: узловой вектор должен быть неубывающим");
            }
        }
        for (Scalar w : m_weights) {
            if (w <= Scalar(0)) {
                throw std::invalid_argument("NurbsCurve: веса должны быть положительными");
            }
        }
    }

    [[nodiscard]] int findSpan(Scalar t) const noexcept {
        int n = static_cast<int>(m_controlPoints.size());
        if (t <= m_knots[m_degree]) return m_degree;
        if (t >= m_knots[n]) return n - 1;

        int low = m_degree;
        int high = n;
        int mid = (low + high) / 2;
        while (t < m_knots[mid] || t >= m_knots[mid + 1]) {
            if (t < m_knots[mid]) high = mid;
            else low = mid;
            mid = (low + high) / 2;
        }
        return mid;
    }

    // Взвешенная сумма контрольных точек с использованием строгого разделения Point3/Vector3
    [[nodiscard]] Point3 weightedSum(const std::vector<Scalar>& N) const noexcept {
        Point3 numerator = Point3::origin();   // или Point3{0,0,0}
        Scalar denominator = Scalar(0);

        for (size_t i = 0; i < m_controlPoints.size(); ++i) {
            Scalar wN = m_weights[i] * N[i];
            // Преобразуем контрольную точку в вектор для умножения на скаляр
            Vector3 cpVec(m_controlPoints[i].x, m_controlPoints[i].y, m_controlPoints[i].z);
            numerator = numerator + (cpVec * wN);
            denominator += wN;
        }

        if (denominator < Scalar(1e-20)) {
            return m_controlPoints.front();
        }

        // Вектор из начала координат в числитель делим на знаменатель,
        // затем прибавляем к началу координат, получая точку
        Vector3 weightedVec = (numerator - Point3::origin()) / denominator;
        return Point3::origin() + weightedVec;
    }
};

} // namespace mir