// MirEngine/Geometry/Curve/BezierCurve.hpp
// 🎨 Кривая Безье — гладкая кривая, заданная набором контрольных точек.
//
// Кривая Безье — это один из самых популярных способов описания плавных
// кривых. Вместо того чтобы задавать каждую точку на кривой, ты задаёшь
// несколько "контрольных" точек — и кривая сама проходит через них,
// огибая их плавным образом. Чем больше контрольных точек, тем сложнее
// форму кривой.
//
// Как это работает (на пальцах):
//   • 2 точки  → прямая линия (линейная Безье).
//   • 3 точки  → квадратичная Безье (парабола).
//   • 4 точки  → кубическая Безье (самая популярная в графике).
//   • n точек  → кривая степени (n-1).
//
// Кривая всегда начинается в первой контрольной точке и заканчивается
// в последней. Промежуточные точки "притягивают" кривую к себе, но она
// через них не проходит.
//
// Использование в CAD:
//   • Контуры эскизов (Sketch).
//   • Траектории для выдавливания и сдвига (Sweep).
//   • Поверхности Безье (из сетки контрольных точек).
//   • Анимации (плавные траектории движения).
//
// Важно: контрольные точки хранятся в 3D, но кривая может лежать и в плоскости.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "ParametricCurve.hpp"       // Базовый класс
#include "../Point/Point3.hpp"       // mir::Point3
#include "../../Core/Types/Scalar.hpp" // mir::Scalar
#include <vector>                    // для хранения контрольных точек
#include <algorithm>                 // std::clamp

namespace mir {

class BezierCurve : public ParametricCurve {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт кривую Безье из вектора контрольных точек.
    // Минимальное количество точек = 2 (линейная кривая).
    explicit BezierCurve(std::vector<Point3> controlPoints) noexcept
        : m_controlPoints(std::move(controlPoints))
    {
        // Гарантируем хотя бы 2 точки.
        if (m_controlPoints.size() < 2) {
            m_controlPoints = {Point3::origin(), Point3{1.0, 0.0, 0.0}};
        }
    }

    // Удобный конструктор для кубической Безье (4 точки).
    BezierCurve(const Point3& p0, const Point3& p1, const Point3& p2, const Point3& p3) noexcept
        : m_controlPoints({p0, p1, p2, p3})
    {}

    // ── Доступ к контрольным точкам ──────────────────────────

    [[nodiscard]] const std::vector<Point3>& controlPoints() const noexcept {
        return m_controlPoints;
    }

    // Количество контрольных точек.
    [[nodiscard]] int degree() const noexcept {
        return static_cast<int>(m_controlPoints.size()) - 1;
    }

    // ── Реализация ParametricCurve ──────────────────────────

    // Точка на кривой при параметре t (алгоритм де Кастельжо).
    [[nodiscard]] Point3 pointAt(Scalar t) const override {
        t = std::clamp(t, Scalar(0), Scalar(1));

        // Алгоритм де Кастельжо: итеративно вычисляем промежуточные точки,
        // пока не останется одна.
        std::vector<Point3> points = m_controlPoints;  // копируем
        int n = static_cast<int>(points.size());

        while (n > 1) {
            for (int i = 0; i < n - 1; ++i) {
                // Линейная интерполяция между соседними точками
                points[i] = Point3::lerp(points[i], points[i + 1], t);
            }
            --n;
        }
        return points[0];
    }

    // Касательный вектор (первая производная).
    [[nodiscard]] Vector3 tangentAt(Scalar t) const override {
        t = std::clamp(t, Scalar(0), Scalar(1));

        // Производная Безье — это тоже кривая Безье на одну степень ниже.
        std::vector<Point3> derivPoints = m_controlPoints;
        int n = static_cast<int>(derivPoints.size());

        if (n < 2) return Vector3::zero();

        // Строим контрольные точки для производной: dP[i] = n * (P[i+1] - P[i])
        std::vector<Vector3> derivVectors;
        for (int i = 0; i < n - 1; ++i) {
            derivVectors.push_back((derivPoints[i + 1] - derivPoints[i]) * Scalar(n - 1));
        }

        // Вычисляем точку на кривой производной (алгоритм де Кастельжо для векторов)
        std::vector<Vector3> temp = derivVectors;
        int m = static_cast<int>(temp.size());
        while (m > 1) {
            for (int i = 0; i < m - 1; ++i) {
                temp[i] = Vector3::lerp(temp[i], temp[i + 1], t);
            }
            --m;
        }
        return temp.empty() ? Vector3::zero() : temp[0];
    }

    // ── Разбиение кривой ────────────────────────────────────

    // Разделяет кривую на две в точке t. Возвращает две новых кривых Безье.
    [[nodiscard]] std::pair<BezierCurve, BezierCurve> split(Scalar t) const noexcept {
        t = std::clamp(t, Scalar(0), Scalar(1));
        int n = static_cast<int>(m_controlPoints.size());
        if (n < 2) {
            return {*this, *this};
        }

        // Алгоритм де Кастельжо: сохраняем все промежуточные точки.
        std::vector<std::vector<Point3>> triangle;
        triangle.push_back(m_controlPoints);

        for (int level = 1; level < n; ++level) {
            std::vector<Point3> next;
            const auto& prev = triangle[level - 1];
            for (size_t i = 0; i < prev.size() - 1; ++i) {
                next.push_back(Point3::lerp(prev[i], prev[i + 1], t));
            }
            triangle.push_back(std::move(next));
        }

        // Левая часть: первые точки каждого уровня
        std::vector<Point3> leftPoints;
        for (int i = 0; i < n; ++i) {
            leftPoints.push_back(triangle[i][0]);
        }

        // Правая часть: последние точки каждого уровня в обратном порядке
        std::vector<Point3> rightPoints;
        for (int i = n - 1; i >= 0; --i) {
            rightPoints.push_back(triangle[i].back());
        }

        return {BezierCurve(leftPoints), BezierCurve(rightPoints)};
    }

    // ── Повышение степени ───────────────────────────────────

    // Возвращает новую кривую на одну степень выше с минимальным изменением формы.
    [[nodiscard]] BezierCurve elevateDegree() const noexcept {
        int n = static_cast<int>(m_controlPoints.size());
        if (n < 2) return *this;

        std::vector<Point3> newPoints;
        newPoints.reserve(n + 1);

        // Первая точка та же
        newPoints.push_back(m_controlPoints[0]);

        // Промежуточные: (i / (n+1)) * P[i-1] + (1 - i / (n+1)) * P[i]
        for (int i = 1; i < n; ++i) {
            Scalar factor = Scalar(i) / Scalar(n);
            newPoints.push_back(Point3::lerp(m_controlPoints[i], m_controlPoints[i - 1], factor));
        }

        // Последняя точка та же
        newPoints.push_back(m_controlPoints.back());

        return BezierCurve(std::move(newPoints));
    }

private:
    std::vector<Point3> m_controlPoints;   // контрольные точки (от P0 до Pn)
};

} // namespace mir