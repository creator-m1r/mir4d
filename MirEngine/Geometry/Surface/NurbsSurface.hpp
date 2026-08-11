// MirEngine/Geometry/Surface/NurbsSurface.hpp
// 🎯 NURBS-поверхность — двумерный аналог NurbsCurve для поверхностей сложной формы.
//
// NURBS-поверхность (Non-Uniform Rational B-Spline Surface) — это
// математически точный способ описать практически любую гладкую поверхность.
// Как и кривая, поверхность задаётся контрольными точками, их весами и
// узловыми векторами, но теперь по двум направлениям — u и v.
//
// Параметризация:
//   • u и v изменяются в диапазонах, определённых узловыми векторами.
//   • pointAt(u, v) возвращает точку на поверхности.
//   • normalAt(u, v) возвращает нормаль (перпендикуляр к поверхности).
//
// Основные возможности:
//   • Вычисление точки и нормали.
//   • Вставка узлов (insertKnotU, insertKnotV).
//   • Повышение степени (elevateDegreeU, elevateDegreeV).
//   • Разбиение на сетку для визуализации.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Surface.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../Point/Point3.hpp"
#include "../Direction/Direction3.hpp"
#include "../../Core/Types/Scalar.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace mir {

class NurbsSurface : public Surface {
public:
    // ── Конструктор ─────────────────────────────────────────
    NurbsSurface(
        std::vector<std::vector<Point3>> controlPoints,
        std::vector<std::vector<Scalar>> weights,
        std::vector<Scalar> knotsU,
        std::vector<Scalar> knotsV,
        int degreeU,
        int degreeV
    )
        : m_controlPoints(std::move(controlPoints))
        , m_weights(std::move(weights))
        , m_knotsU(std::move(knotsU))
        , m_knotsV(std::move(knotsV))
        , m_degreeU(degreeU)
        , m_degreeV(degreeV)
    {
        validate();
    }

    // ── Доступ к данным ─────────────────────────────────────
    [[nodiscard]] const auto& controlPoints() const noexcept { return m_controlPoints; }
    [[nodiscard]] const auto& weights()       const noexcept { return m_weights; }
    [[nodiscard]] const auto& knotsU()        const noexcept { return m_knotsU; }
    [[nodiscard]] const auto& knotsV()        const noexcept { return m_knotsV; }
    [[nodiscard]] int degreeU() const noexcept { return m_degreeU; }
    [[nodiscard]] int degreeV() const noexcept { return m_degreeV; }
    [[nodiscard]] int numControlPointsU() const noexcept { return static_cast<int>(m_controlPoints.empty() ? 0 : m_controlPoints[0].size()); }
    [[nodiscard]] int numControlPointsV() const noexcept { return static_cast<int>(m_controlPoints.size()); }

    // ── Точка на поверхности ───────────────────────────────
    [[nodiscard]] Point3 pointAt(Scalar u, Scalar v) const override {
        u = std::clamp(u, m_knotsU.front(), m_knotsU.back() - Scalar(1e-10));
        v = std::clamp(v, m_knotsV.front(), m_knotsV.back() - Scalar(1e-10));
        auto Nu = basisFunctions(m_knotsU, m_degreeU, u, numControlPointsU());
        auto Nv = basisFunctions(m_knotsV, m_degreeV, v, numControlPointsV());
        return weightedSum(Nu, Nv);
    }

    // ── Нормаль в точке ─────────────────────────────────────
    [[nodiscard]] Direction3 normalAt(Scalar u, Scalar v) const override {
        Vector3 dU = derivativeU(u, v);
        Vector3 dV = derivativeV(u, v);
        Vector3 n = Vector3::cross(dU, dV);
        return Direction3::fromVector(n);
    }

    // ── Частные производные ─────────────────────────────────
    [[nodiscard]] Vector3 derivativeU(Scalar u, Scalar v) const override {
        u = std::clamp(u, m_knotsU.front(), m_knotsU.back() - Scalar(1e-10));
        v = std::clamp(v, m_knotsV.front(), m_knotsV.back() - Scalar(1e-10));
        auto Nu = basisFunctionDerivatives(m_knotsU, m_degreeU, u, numControlPointsU());
        auto Nv = basisFunctions(m_knotsV, m_degreeV, v, numControlPointsV());
        // Производная по u — вектор, получаем его вычитанием начала координат из полученной точки
        return weightedSum(Nu, Nv) - Point3::origin();
    }

    [[nodiscard]] Vector3 derivativeV(Scalar u, Scalar v) const override {
        u = std::clamp(u, m_knotsU.front(), m_knotsU.back() - Scalar(1e-10));
        v = std::clamp(v, m_knotsV.front(), m_knotsV.back() - Scalar(1e-10));
        auto Nu = basisFunctions(m_knotsU, m_degreeU, u, numControlPointsU());
        auto Nv = basisFunctionDerivatives(m_knotsV, m_degreeV, v, numControlPointsV());
        return weightedSum(Nu, Nv) - Point3::origin();
    }

    // ── Вставка узлов ───────────────────────────────────────
    void insertKnotU(Scalar u, int times = 1) noexcept {
        for (int r = 0; r < times; ++r) {
            insertKnot(m_knotsU, m_degreeU, u, true);
        }
    }

    void insertKnotV(Scalar v, int times = 1) noexcept {
        for (int r = 0; r < times; ++r) {
            insertKnot(m_knotsV, m_degreeV, v, false);
        }
    }

    // ── Повышение степени ───────────────────────────────────
    void elevateDegreeU() noexcept {
        elevateDegree(m_knotsU, m_degreeU, true);
    }

    void elevateDegreeV() noexcept {
        elevateDegree(m_knotsV, m_degreeV, false);
    }

private:
    std::vector<std::vector<Point3>> m_controlPoints; // [v][u]
    std::vector<std::vector<Scalar>> m_weights;       // [v][u]
    std::vector<Scalar> m_knotsU;
    std::vector<Scalar> m_knotsV;
    int m_degreeU;
    int m_degreeV;

    void validate() {
        if (m_controlPoints.empty() || m_controlPoints[0].empty()) {
            throw std::invalid_argument("NurbsSurface: контрольные точки не могут быть пустыми");
        }
        int numU = static_cast<int>(m_controlPoints[0].size());
        int numV = static_cast<int>(m_controlPoints.size());
        if (numU < m_degreeU + 1 || numV < m_degreeV + 1) {
            throw std::invalid_argument("NurbsSurface: недостаточно контрольных точек");
        }
        if (m_weights.size() != static_cast<size_t>(numV) || 
            std::any_of(m_weights.begin(), m_weights.end(), [numU](const auto& row) { return row.size() != static_cast<size_t>(numU); })) {
            throw std::invalid_argument("NurbsSurface: размер сетки весов не совпадает с контрольными точками");
        }
        if (m_knotsU.size() != static_cast<size_t>(numU + m_degreeU + 1) ||
            m_knotsV.size() != static_cast<size_t>(numV + m_degreeV + 1)) {
            throw std::invalid_argument("NurbsSurface: неверный размер узлового вектора");
        }
        for (size_t i = 1; i < m_knotsU.size(); ++i)
            if (m_knotsU[i] < m_knotsU[i-1]) throw std::invalid_argument("NurbsSurface: knotsU не упорядочены");
        for (size_t i = 1; i < m_knotsV.size(); ++i)
            if (m_knotsV[i] < m_knotsV[i-1]) throw std::invalid_argument("NurbsSurface: knotsV не упорядочены");
        for (const auto& row : m_weights)
            for (Scalar w : row)
                if (w <= Scalar(0)) throw std::invalid_argument("NurbsSurface: веса должны быть > 0");
    }

    static std::vector<Scalar> basisFunctions(
        const std::vector<Scalar>& knots, int degree, Scalar t, int numPoints
    ) noexcept {
        std::vector<Scalar> N(numPoints, Scalar(0));
        if (t <= knots[degree]) {
            N[degree] = Scalar(1);
            return N;
        }
        if (t >= knots[numPoints]) {
            N[numPoints - 1] = Scalar(1);
            return N;
        }

        int span = degree;
        for (int i = degree; i < numPoints; ++i) {
            if (t >= knots[i] && t < knots[i+1]) {
                span = i;
                break;
            }
        }

        N[span] = Scalar(1);
        for (int d = 1; d <= degree; ++d) {
            Scalar left = t - knots[span - d + 1];
            Scalar denomLeft = knots[span + 1] - knots[span - d + 1];
            Scalar saved = Scalar(0);
            if (denomLeft > Scalar(0))
                N[span - d] = left / denomLeft * N[span - d + 1];
            for (int i = span - d + 1; i <= span - 1; ++i) {
                Scalar left2 = t - knots[i];
                Scalar denomLeft2 = knots[i + d] - knots[i];
                Scalar right2 = knots[i + d + 1] - t;
                Scalar denomRight2 = knots[i + d + 1] - knots[i + 1];
                Scalar term = Scalar(0);
                if (denomLeft2 > Scalar(0)) term += left2 / denomLeft2 * N[i];
                if (denomRight2 > Scalar(0)) term += right2 / denomRight2 * N[i + 1];
                N[i] = term;
            }
            Scalar right = knots[span + d + 1] - t;
            Scalar denomRight = knots[span + d + 1] - knots[span + 1];
            if (denomRight > Scalar(0))
                N[span] = right / denomRight * N[span];
        }
        return N;
    }

    static std::vector<Scalar> basisFunctionDerivatives(
        const std::vector<Scalar>& knots, int degree, Scalar t, int numPoints
    ) noexcept {
        std::vector<Scalar> dN(numPoints, Scalar(0));
        if (degree < 1) return dN;

        std::vector<Scalar> N_low = basisFunctions(knots, degree - 1, t, numPoints + 1);
        for (int i = 0; i < numPoints; ++i) {
            Scalar denom1 = knots[i + degree] - knots[i];
            Scalar term1 = (denom1 > Scalar(0)) ? (degree * N_low[i] / denom1) : Scalar(0);
            Scalar denom2 = knots[i + degree + 1] - knots[i + 1];
            Scalar term2 = (denom2 > Scalar(0)) ? (degree * N_low[i + 1] / denom2) : Scalar(0);
            dN[i] = term1 - term2;
        }
        return dN;
    }

    // ── Взвешенная сумма контрольных точек (возвращает Point3) ─
    [[nodiscard]] Point3 weightedSum(
        const std::vector<Scalar>& Nu, const std::vector<Scalar>& Nv
    ) const noexcept {
        Point3 numerator = Point3::origin();
        Scalar denominator = Scalar(0);
        int numV = numControlPointsV();
        int numU = numControlPointsU();
        for (int j = 0; j < numV; ++j) {
            for (int i = 0; i < numU; ++i) {
                Scalar wN = m_weights[j][i] * Nu[i] * Nv[j];
                // Преобразуем контрольную точку в вектор для умножения на скаляр и сложения с точкой
                Vector3 cpVec(m_controlPoints[j][i].x, m_controlPoints[j][i].y, m_controlPoints[j][i].z);
                numerator = numerator + (cpVec * wN);
                denominator += wN;
            }
        }
        if (denominator < Scalar(1e-20)) {
            return m_controlPoints.front().front();
        }
        // (numerator - origin) даёт вектор, делим на скаляр и прибавляем origin
        return Point3::origin() + (numerator - Point3::origin()) / denominator;
    }

    // ── Вставка одного узла ──────────────────────────────────
    void insertKnot(std::vector<Scalar>& knots, int degree, Scalar t, bool isU) noexcept {
        int numPoints = isU ? numControlPointsU() : numControlPointsV();
        int span = 0;
        for (int i = degree; i < numPoints; ++i) {
            if (t >= knots[i] && t < knots[i+1]) { span = i; break; }
        }

        if (isU) {
            int numV = numControlPointsV();
            std::vector<std::vector<Point3>> newCPs(numV, std::vector<Point3>(numPoints + 1));
            std::vector<std::vector<Scalar>> newWeights(numV, std::vector<Scalar>(numPoints + 1));
            for (int j = 0; j < numV; ++j) {
                for (int i = 0; i <= span - degree; ++i) {
                    newCPs[j][i] = m_controlPoints[j][i];
                    newWeights[j][i] = m_weights[j][i];
                }
                for (int i = span; i < numPoints; ++i) {
                    newCPs[j][i + 1] = m_controlPoints[j][i];
                    newWeights[j][i + 1] = m_weights[j][i];
                }
                for (int i = span - degree + 1; i <= span; ++i) {
                    Scalar alpha = (t - knots[i]) / (knots[i + degree] - knots[i]);
                    alpha = std::clamp(alpha, Scalar(0), Scalar(1));
                    newWeights[j][i] = (Scalar(1) - alpha) * m_weights[j][i - 1] + alpha * m_weights[j][i];
                    newCPs[j][i] = Point3::lerp(m_controlPoints[j][i - 1], m_controlPoints[j][i], alpha);
                }
            }
            m_controlPoints = std::move(newCPs);
            m_weights = std::move(newWeights);
        } else {
            int numU = numControlPointsU();
            int numV = numControlPointsV();
            std::vector<std::vector<Point3>> newCPs(numV + 1, std::vector<Point3>(numU));
            std::vector<std::vector<Scalar>> newWeights(numV + 1, std::vector<Scalar>(numU));
            for (int i = 0; i < numU; ++i) {
                for (int j = 0; j <= span - degree; ++j) {
                    newCPs[j][i] = m_controlPoints[j][i];
                    newWeights[j][i] = m_weights[j][i];
                }
                for (int j = span; j < numV; ++j) {
                    newCPs[j + 1][i] = m_controlPoints[j][i];
                    newWeights[j + 1][i] = m_weights[j][i];
                }
                for (int j = span - degree + 1; j <= span; ++j) {
                    Scalar alpha = (t - knots[j]) / (knots[j + degree] - knots[j]);
                    alpha = std::clamp(alpha, Scalar(0), Scalar(1));
                    newWeights[j][i] = (Scalar(1) - alpha) * m_weights[j - 1][i] + alpha * m_weights[j][i];
                    newCPs[j][i] = Point3::lerp(m_controlPoints[j - 1][i], m_controlPoints[j][i], alpha);
                }
            }
            m_controlPoints = std::move(newCPs);
            m_weights = std::move(newWeights);
        }

        std::vector<Scalar> newKnots(knots.size() + 1);
        for (int i = 0; i <= span; ++i) newKnots[i] = knots[i];
        newKnots[span + 1] = t;
        for (size_t i = span + 1; i < knots.size(); ++i) newKnots[i + 1] = knots[i];
        knots = std::move(newKnots);
    }

    // ── Повышение степени ────────────────────────────────────
    void elevateDegree(std::vector<Scalar>& knots, int& degree, bool isU) noexcept {
        int numPoints = isU ? numControlPointsU() : numControlPointsV();
        int otherNum = isU ? numControlPointsV() : numControlPointsU();
        int newDegree = degree + 1;
        std::vector<Scalar> newKnots = { knots.front() };
        for (size_t i = 1; i < knots.size() - 1; ++i) {
            newKnots.push_back(knots[i]);
            newKnots.push_back(knots[i]);
        }
        newKnots.push_back(knots.back());

        if (isU) {
            int numV = otherNum;
            std::vector<std::vector<Point3>> newCPs(numV, std::vector<Point3>(numPoints + 1));
            std::vector<std::vector<Scalar>> newWeights(numV, std::vector<Scalar>(numPoints + 1));
            for (int j = 0; j < numV; ++j) {
                newCPs[j][0] = m_controlPoints[j][0];
                newWeights[j][0] = m_weights[j][0];
                for (int i = 1; i < numPoints; ++i) {
                    Scalar alpha = Scalar(i) / Scalar(newDegree);
                    newCPs[j][i] = Point3::lerp(m_controlPoints[j][i], m_controlPoints[j][i - 1], alpha);
                    newWeights[j][i] = (Scalar(1) - alpha) * m_weights[j][i] + alpha * m_weights[j][i - 1];
                }
                newCPs[j][numPoints] = m_controlPoints[j][numPoints - 1];
                newWeights[j][numPoints] = m_weights[j][numPoints - 1];
            }
            m_controlPoints = std::move(newCPs);
            m_weights = std::move(newWeights);
        } else {
            int numU = otherNum;
            int numV = numPoints;
            std::vector<std::vector<Point3>> newCPs(numV + 1, std::vector<Point3>(numU));
            std::vector<std::vector<Scalar>> newWeights(numV + 1, std::vector<Scalar>(numU));
            for (int i = 0; i < numU; ++i) {
                newCPs[0][i] = m_controlPoints[0][i];
                newWeights[0][i] = m_weights[0][i];
                for (int j = 1; j < numV; ++j) {
                    Scalar alpha = Scalar(j) / Scalar(newDegree);
                    newCPs[j][i] = Point3::lerp(m_controlPoints[j][i], m_controlPoints[j - 1][i], alpha);
                    newWeights[j][i] = (Scalar(1) - alpha) * m_weights[j][i] + alpha * m_weights[j - 1][i];
                }
                newCPs[numV][i] = m_controlPoints[numV - 1][i];
                newWeights[numV][i] = m_weights[numV - 1][i];
            }
            m_controlPoints = std::move(newCPs);
            m_weights = std::move(newWeights);
        }
        knots = std::move(newKnots);
        degree = newDegree;
    }
};

} // namespace mir