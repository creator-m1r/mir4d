// MirEngine/Geometry/Point/Point3.hpp
// 📍 Точка в трёхмерном пространстве — фундаментальный геометрический примитив.
//
// Точка — это самое простое, с чего начинается вся геометрия.
// Она не имеет размера, направления или объёма — только положение в пространстве,
// заданное тремя координатами (x, y, z).
//
// В MirEngine точка и вектор — это разные типы, хотя оба содержат три числа.
// Почему? Потому что с математической точки зрения они ведут себя по-разному:
//   • Точка + Вектор = Точка     (сдвинули точку)
//   • Точка - Точка   = Вектор   (расстояние и направление между точками)
//   • Точка + Точка   = ❌ ошибка (бессмысленно)
//   • Вектор + Вектор = Вектор   (сумма смещений)
//   • Вектор * Скаляр = Вектор   (удлинили направление)
//
// Разделяя эти типы, мы делаем код самодокументированным:
// глядя на сигнатуру функции, сразу понятно, что она принимает —
// координату (Point3) или направление/смещение (Vector3).
//
// Point3 используется:
//   • Для хранения координат вершин, позиций объектов.
//   • В качестве опорных точек при построении кривых и поверхностей.
//   • Для задания начала и конца отрезков, лучей.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <cmath>

#include "../Vector/Vector3.hpp"

namespace mir
{

class Point3
{
public:

    using Scalar = double;

    Scalar x = 0.0;
    Scalar y = 0.0;
    Scalar z = 0.0;

    // --------------------------------------------------------
    // Constructors
    // --------------------------------------------------------

    constexpr Point3() noexcept = default;

    constexpr Point3(
        Scalar x_,
        Scalar y_,
        Scalar z_) noexcept
        : x(x_)
        , y(y_)
        , z(z_)
    {
    }

    // --------------------------------------------------------
    // Point + Vector = Point
    // --------------------------------------------------------

    constexpr Point3 operator+(
        const Vector3& vector) const noexcept
    {
        return Point3{
            x + vector.x,
            y + vector.y,
            z + vector.z
        };
    }

    // --------------------------------------------------------
    // Point - Vector = Point
    // --------------------------------------------------------

    constexpr Point3 operator-(
        const Vector3& vector) const noexcept
    {
        return Point3{
            x - vector.x,
            y - vector.y,
            z - vector.z
        };
    }

    // --------------------------------------------------------
    // Point - Point = Vector
    // --------------------------------------------------------

    constexpr Vector3 operator-(
        const Point3& other) const noexcept
    {
        return Vector3{
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    // --------------------------------------------------------
    // Comparison
    // --------------------------------------------------------

    constexpr bool operator==(
        const Point3& other) const noexcept
    {
        return x == other.x &&
               y == other.y &&
               z == other.z;
    }

    constexpr bool operator!=(
        const Point3& other) const noexcept
    {
        return !(*this == other);
    }

    // --------------------------------------------------------
    // Distance
    // --------------------------------------------------------

    constexpr Scalar squaredDistance(
        const Point3& other) const noexcept
    {
        const Scalar dx = x - other.x;
        const Scalar dy = y - other.y;
        const Scalar dz = z - other.z;

        return
            dx * dx +
            dy * dy +
            dz * dz;
    }

    Scalar distance(
        const Point3& other) const noexcept
    {
        return std::sqrt(
            squaredDistance(other)
        );
    }

    // --------------------------------------------------------
    // Translation
    // --------------------------------------------------------

    void translate(
        const Vector3& delta) noexcept
    {
        x += delta.x;
        y += delta.y;
        z += delta.z;
    }

    // --------------------------------------------------------
    // Origin
    // --------------------------------------------------------

    static constexpr Point3 origin() noexcept
    {
        return Point3{
            0.0,
            0.0,
            0.0
        };
    }
};

} // namespace mir