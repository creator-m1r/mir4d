// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Geometry/Bounding/BoundingSphere.hpp
// ─────────────────────────────────────────────────────────────
// 🎈 ОГРАНИЧИВАЮЩАЯ СФЕРА (Bounding Sphere) — невидимый шар вокруг объекта
//
// Представь, что ты хочешь упаковать хрупкую игрушку в шар из
// пенопласта, чтобы она не разбилась при перевозке. Шар должен
// быть как можно меньше, но полностью содержать игрушку.
//
// Такой шар хорош тем, что:
//   1. Он одинаковый со всех сторон — неважно, как повёрнут объект.
//   2. Проверка столкновения двух шаров — одна из самых быстрых
//      в компьютерной графике: просто вычисли расстояние между
//      центрами и сравни с суммой радиусов.
//
// 📐 Как задаётся сфера?
//   Центр (center) + радиус (radius).
//   В 2D это круг, в 3D — полноценная сфера, в 4D — гиперсфера.
//
// 📌 Где используется?
//   • Быстрое отсечение объектов за пределами камеры (frustum culling).
//   • Первый проход проверки столкновений (broad phase).
//   • Быстрый поиск ближайших объектов.
//
// Чистый C++23, без внешних зависимостей.
// ─────────────────────────────────────────────────────────────

#pragma once

#include "../Math/Vector/Vector2.hpp"      // Vec2
#include "../Math/Vector/Vector3.hpp"      // Vec3
#include "../Math/Vector/Vector4.hpp"      // Vec4
#include <vector>                          // std::vector
#include <cmath>                           // std::sqrt, std::fabs
#include <limits>                          // std::numeric_limits
#include <algorithm>                       // std::max

namespace M1R {

// ╔══════════════════════════════════════════════════════════╗
// ║  Шаблонный класс BoundingSphere<VectorType>              ║
// ╚══════════════════════════════════════════════════════════╝
template<typename VectorType>
class BoundingSphere {
public:
    using Scalar = decltype(VectorType::x);   // float или double

    VectorType center;      // 🎯 центр сферы (точка в пространстве)
    Scalar     radius;      // 📏 радиус сферы (половина диаметра)

    // ─────────────────────────────────────────────────────────
    // 1️⃣ Конструктор: пустая сфера (центр в нуле, радиус = 0)
    // ─────────────────────────────────────────────────────────
    BoundingSphere() : center{}, radius(0) {}

    // ─────────────────────────────────────────────────────────
    // 2️⃣ Конструктор от центра и радиуса
    //    Пример: BoundingSphere(Vec3(0,0,0), 5.0)
    //    создаст шар с центром в начале координат и радиусом 5.
    // ─────────────────────────────────────────────────────────
    BoundingSphere(const VectorType& center_, Scalar radius_)
        : center(center_), radius(radius_) {}

    // ─────────────────────────────────────────────────────────
    // 3️⃣ Создать сферу из набора точек (минимальная содержащая)
    //
    //    Алгоритм Велзла (Welzl) был бы точнее, но для наших
    //    инженерных целей мы используем быстрый и простой метод:
    //    центр = среднее арифметическое всех точек,
    //    радиус = максимальное расстояние от центра до любой точки.
    // ─────────────────────────────────────────────────────────
    static BoundingSphere fromPoints(const std::vector<VectorType>& points) {
        if (points.empty()) {
            return BoundingSphere();   // пустая сфера
        }

        // Находим среднее арифметическое (центр)
        VectorType sum = points[0];
        for (size_t i = 1; i < points.size(); ++i) {
            sum = sum + points[i];
        }
        VectorType center = sum / Scalar(points.size());

        // Находим максимальное расстояние до центра
        Scalar maxDistSq = 0;
        for (const auto& p : points) {
            VectorType diff = p - center;
            Scalar distSq = diff.x * diff.x + diff.y * diff.y
                          + diff.z * diff.z + diff.w * diff.w;
            if (distSq > maxDistSq) {
                maxDistSq = distSq;
            }
        }

        return BoundingSphere(center, std::sqrt(maxDistSq));
    }

    // ─────────────────────────────────────────────────────────
    // 4️⃣ Проверка: пустая ли сфера?
    //    Пустая, если радиус равен нулю.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] bool isEmpty() const {
        return radius <= Scalar(0);
    }

    // ─────────────────────────────────────────────────────────
    // 5️⃣ Содержит ли сфера ТОЧКУ?
    //    Точка внутри, если расстояние от центра до точки ≤ радиус.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] bool contains(const VectorType& point) const {
        VectorType diff = point - center;
        Scalar distSq = diff.x * diff.x + diff.y * diff.y
                      + diff.z * diff.z + diff.w * diff.w;
        return distSq <= radius * radius;
    }

    // ─────────────────────────────────────────────────────────
    // 6️⃣ Пересекается ли сфера с другой сферой?
    //    Сферы пересекаются, если расстояние между их центрами
    //    меньше или равно сумме радиусов.
    //
    //    🎱 Пример: два бильярдных шара касаются, когда расстояние
    //    между их центрами равно сумме радиусов.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] bool intersects(const BoundingSphere& other) const {
        VectorType diff = center - other.center;
        Scalar distSq = diff.x * diff.x + diff.y * diff.y
                      + diff.z * diff.z + diff.w * diff.w;
        Scalar sumRadius = radius + other.radius;
        return distSq <= sumRadius * sumRadius;
    }

    // ─────────────────────────────────────────────────────────
    // 7️⃣ Содержит ли сфера другую сферу ЦЕЛИКОМ?
    //    Другая сфера полностью внутри, если расстояние от её центра
    //    до нашего центра + её радиус ≤ наш радиус.
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] bool contains(const BoundingSphere& other) const {
        VectorType diff = center - other.center;
        Scalar dist = std::sqrt(diff.x * diff.x + diff.y * diff.y
                              + diff.z * diff.z + diff.w * diff.w);
        return (dist + other.radius) <= radius;
    }

    // ─────────────────────────────────────────────────────────
    // 8️⃣ Расширить сферу, чтобы включить ТОЧКУ
    //    Если точка снаружи, увеличиваем радиус так, чтобы она
    //    оказалась на границе новой сферы. Центр при этом не меняется,
    //    поэтому сфера может стать не минимальной, но метод — быстрый.
    // ─────────────────────────────────────────────────────────
    void extend(const VectorType& point) {
        VectorType diff = point - center;
        Scalar dist = std::sqrt(diff.x * diff.x + diff.y * diff.y
                              + diff.z * diff.z + diff.w * diff.w);
        if (dist > radius) {
            radius = dist;
        }
    }

    // ─────────────────────────────────────────────────────────
    // 9️⃣ Расширить сферу, чтобы включить другую СФЕРУ
    //    Вычисляем расстояние между центрами плюс радиус другой сферы.
    //    Если это больше текущего радиуса, увеличиваем до него.
    // ─────────────────────────────────────────────────────────
    void extend(const BoundingSphere& other) {
        if (other.isEmpty()) return;

        VectorType diff = center - other.center;
        Scalar dist = std::sqrt(diff.x * diff.x + diff.y * diff.y
                              + diff.z * diff.z + diff.w * diff.w);
        Scalar newRadius = dist + other.radius;
        if (newRadius > radius) {
            radius = newRadius;
        }
    }

    // ─────────────────────────────────────────────────────────
    // 🔟 Диаметр сферы (удобный метод)
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] Scalar diameter() const {
        return radius * Scalar(2);
    }

    // ─────────────────────────────────────────────────────────
    // 1️⃣1️⃣ Длина окружности / площадь поверхности / объём
    //        Для 3D: площадь поверхности = 4πr², объём = ⁴⁄₃πr³
    //        Для 2D: длина окружности = 2πr, площадь = πr²
    // ─────────────────────────────────────────────────────────
    [[nodiscard]] Scalar circumference() const {
        return Scalar(2) * Scalar(3.14159265358979323846) * radius;
    }

    [[nodiscard]] Scalar surfaceArea() const {
        return Scalar(4) * Scalar(3.14159265358979323846) * radius * radius;
    }

    [[nodiscard]] Scalar volume() const {
        return Scalar(4.0 / 3.0) * Scalar(3.14159265358979323846)
               * radius * radius * radius;
    }

    // ─────────────────────────────────────────────────────────
    // 1️⃣2️⃣ Равны ли две сферы?
    // ─────────────────────────────────────────────────────────
    bool operator==(const BoundingSphere& other) const {
        return center == other.center && radius == other.radius;
    }
    bool operator!=(const BoundingSphere& other) const {
        return !(*this == other);
    }
};

} // namespace M1R