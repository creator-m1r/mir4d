// MirEngine/Math/Plane.hpp
// 📐 Математическая плоскость — хранит нормаль и смещение, предоставляет
//    основные операции: расстояние, проекция, пересечение.
//
// В отличие от геометрической плоскости (Geometry/Plane), которая является
// полноценным объектом сцены и может содержать ссылки на Direction3, Line3 и т.д.,
// эта математическая плоскость — лёгкая структура, используемая во внутренних
// вычислениях (например, в алгоритмах отсечения, коллизиях, построении
// ограничивающих объёмов). Она не зависит от других геометрических примитивов
// и может быть быстро создана и уничтожена.
//
// Представление: нормаль (nx, ny, nz) и расстояние d от начала координат
// до плоскости вдоль нормали. Уравнение плоскости: nx*x + ny*y + nz*z + d = 0.
// (Знак d может быть разным в зависимости от соглашения; здесь d = -dot(normal, pointOnPlane)).
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Vector/Vector3.hpp"   // mir::Vector3
#include "../Core/Types/Scalar.hpp" // mir::Scalar

namespace mir {

class Plane {
public:
    // ── Компоненты ───────────────────────────────────────────
    Vector3 normal;   // единичный вектор нормали (nx, ny, nz)
    Scalar  d;        // смещение (d = -dot(normal, pointOnPlane))

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Plane() noexcept : normal(0, 0, 1), d(0) {}

    // Создаёт плоскость по нормали и точке, лежащей на плоскости.
    constexpr Plane(const Vector3& n, const Vector3& point) noexcept
        : normal(n.normalized())
        , d(-Vector3::dot(normal, point))
    {}

    // Создаёт плоскость по трём точкам (не коллинеарным).
    static Plane fromPoints(const Vector3& p1, const Vector3& p2, const Vector3& p3) noexcept {
        Vector3 n = Vector3::cross(p2 - p1, p3 - p1).normalized();
        return Plane(n, p1);
    }

    // ── Расстояние от точки до плоскости ────────────────────
    // Положительное — точка находится со стороны нормали, отрицательное — с противоположной.
    [[nodiscard]] Scalar signedDistance(const Vector3& point) const noexcept {
        return Vector3::dot(normal, point) + d;
    }

    // Абсолютное расстояние (всегда ≥ 0).
    [[nodiscard]] Scalar distance(const Vector3& point) const noexcept {
        return std::abs(signedDistance(point));
    }

    // ── Проекция точки на плоскость ─────────────────────────
    [[nodiscard]] Vector3 project(const Vector3& point) const noexcept {
        return point - normal * signedDistance(point);
    }

    // ── Проверка стороны ────────────────────────────────────
    [[nodiscard]] bool isOnPositiveSide(const Vector3& point, Scalar tolerance = Scalar(1e-10)) const noexcept {
        return signedDistance(point) > tolerance;
    }

    // ── Нормализация ────────────────────────────────────────
    // Убеждаемся, что нормаль единичная (по построению уже так, но на всякий случай).
    void normalize() noexcept {
        Scalar len = normal.length();
        if (len > Scalar(1e-20)) {
            normal /= len;
            d /= len;
        }
    }

    // ── Сравнение ───────────────────────────────────────────
    friend constexpr bool operator==(const Plane& a, const Plane& b) noexcept {
        return a.normal == b.normal && a.d == b.d;
    }
    friend constexpr bool operator!=(const Plane& a, const Plane& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir