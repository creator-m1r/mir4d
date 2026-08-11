// MirEngine/Geometry/Surface/Surface.hpp
// 🧩 Базовый класс для всех поверхностей в MirEngine.
//
// Поверхность — это двумерный геометрический объект, который можно
// представить как "гибкий лист", изогнутый в трёхмерном пространстве.
// В отличие от кривой, которая задаётся одним параметром t, поверхность
// задаётся двумя параметрами (u, v). Когда u и v пробегают от 0 до 1,
// точка на поверхности описывает всю её форму.
//
// Этот абстрактный класс определяет общий интерфейс для всех поверхностей:
//   • pointAt(u, v) — точка на поверхности по параметрам.
//   • normalAt(u, v) — нормаль (вектор "вверх") в заданной точке.
//   • derivativeU / derivativeV — частные производные (касательные).
//   • closestPoint(p) — ближайшая точка на поверхности к заданной.
//   • intersect(ray) — пересечение с лучом (нужно для трассировки лучей).
//   • contains(p) — проверка принадлежности точки поверхности.
//   • tessellate(nu, nv) — сетка точек для отображения.
//
// Конкретные поверхности (PlaneSurface, CylindricalSurface, NurbsSurface...)
// наследуются от этого класса и реализуют эти методы для своей геометрии.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../Direction/Direction3.hpp"   // mir::Direction3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../Ray/Ray3.hpp"              // mir::Ray3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <vector>
#include <optional>
#include <cmath>

namespace mir {

class Surface {
public:
    virtual ~Surface() = default;

    // ── Основные методы (должны быть реализованы в наследниках) ──

    // Точка на поверхности при параметрах (u, v).
    // Обычно u и v ∈ [0, 1], но для бесконечных поверхностей (плоскость)
    // параметры могут быть неограничены.
    [[nodiscard]] virtual Point3 pointAt(Scalar u, Scalar v) const = 0;

    // Нормаль к поверхности в точке (u, v) (единичная, наружу).
    [[nodiscard]] virtual Direction3 normalAt(Scalar u, Scalar v) const {
        Vector3 du = derivativeU(u, v);
        Vector3 dv = derivativeV(u, v);
        Vector3 n = Vector3::cross(du, dv);
        return Direction3::fromVector(n);
    }

    // Частная производная по u (касательный вектор вдоль u).
    [[nodiscard]] virtual Vector3 derivativeU(Scalar u, Scalar v) const {
        Scalar h = Scalar(1e-6);
        return (pointAt(u + h, v) - pointAt(u - h, v)) / (Scalar(2) * h);
    }

    // Частная производная по v.
    [[nodiscard]] virtual Vector3 derivativeV(Scalar u, Scalar v) const {
        Scalar h = Scalar(1e-6);
        return (pointAt(u, v + h) - pointAt(u, v - h)) / (Scalar(2) * h);
    }

    // ── Ближайшая точка на поверхности ──────────────────────
    // Возвращает параметры (u, v), соответствующие ближайшей точке
    // к заданной point. Реализация по умолчанию — поиск по сетке,
    // наследники могут переопределить для аналитического решения.
    [[nodiscard]] virtual std::pair<Scalar, Scalar> closestParameters(
        const Point3& point, int samplesU = 20, int samplesV = 20) const noexcept
    {
        Scalar bestU = Scalar(0), bestV = Scalar(0);
        Scalar bestDist = std::numeric_limits<Scalar>::max();

        for (int i = 0; i <= samplesU; ++i) {
            Scalar u = Scalar(i) / Scalar(samplesU);
            for (int j = 0; j <= samplesV; ++j) {
                Scalar v = Scalar(j) / Scalar(samplesV);
                Scalar d = Point3::distanceSquared(point, pointAt(u, v));
                if (d < bestDist) {
                    bestDist = d;
                    bestU = u;
                    bestV = v;
                }
            }
        }

        // Уточнение методом покоординатного спуска (простая версия)
        Scalar stepU = Scalar(0.5) / Scalar(samplesU);
        Scalar stepV = Scalar(0.5) / Scalar(samplesV);
        for (int iter = 0; iter < 10; ++iter) {
            bool improved = false;
            for (Scalar du = -stepU; du <= stepU; du += stepU * Scalar(2)) {
                for (Scalar dv = -stepV; dv <= stepV; dv += stepV * Scalar(2)) {
                    Scalar nu = bestU + du;
                    Scalar nv = bestV + dv;
                    Scalar d = Point3::distanceSquared(point, pointAt(nu, nv));
                    if (d < bestDist) {
                        bestDist = d;
                        bestU = nu;
                        bestV = nv;
                        improved = true;
                    }
                }
            }
            if (!improved) break;
            stepU *= Scalar(0.5);
            stepV *= Scalar(0.5);
        }
        return {bestU, bestV};
    }

    // Ближайшая точка на поверхности к заданной.
    [[nodiscard]] Point3 closestPoint(const Point3& point,
                                      int samplesU = 20, int samplesV = 20) const noexcept {
        auto [u, v] = closestParameters(point, samplesU, samplesV);
        return pointAt(u, v);
    }

    // ── Проверка принадлежности точки поверхности ───────────
    [[nodiscard]] virtual bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        auto [u, v] = closestParameters(point);
        return Point3::distance(point, pointAt(u, v)) <= tolerance;
    }

    // ── Пересечение с лучом ─────────────────────────────────
    // Возвращает точку пересечения и параметры (u, v), если луч пересекает поверхность.
    // По умолчанию — заглушка, наследники могут реализовать аналитически.
    [[nodiscard]] virtual std::optional<std::tuple<Point3, Scalar, Scalar>> intersect(
        const Ray3& /*ray*/, Scalar /*tolerance*/ = Scalar(1e-10)) const noexcept {
        return std::nullopt;
    }

    // ── Разбиение на сетку ──────────────────────────────────
    // Возвращает сетку точек (nu+1) × (nv+1) для отображения поверхности.
    [[nodiscard]] std::vector<std::vector<Point3>> tessellate(
        int numSegmentsU = 20, int numSegmentsV = 20,
        Scalar uMin = Scalar(0), Scalar uMax = Scalar(1),
        Scalar vMin = Scalar(0), Scalar vMax = Scalar(1)) const noexcept
    {
        std::vector<std::vector<Point3>> grid;
        grid.reserve(numSegmentsU + 1);
        for (int i = 0; i <= numSegmentsU; ++i) {
            Scalar u = uMin + (uMax - uMin) * Scalar(i) / Scalar(numSegmentsU);
            std::vector<Point3> row;
            row.reserve(numSegmentsV + 1);
            for (int j = 0; j <= numSegmentsV; ++j) {
                Scalar v = vMin + (vMax - vMin) * Scalar(j) / Scalar(numSegmentsV);
                row.push_back(pointAt(u, v));
            }
            grid.push_back(std::move(row));
        }
        return grid;
    }
};

} // namespace mir