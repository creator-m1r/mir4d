// MirEngine/Math/Bounds/AABB.hpp
// 📦 Выровненная по осям ограничивающая рамка (Axis-Aligned Bounding Box).
//
// AABB — это самый простой и быстрый способ описать "габариты" объекта.
// Представь прямоугольный ящик, стенки которого строго параллельны осям
// координат X, Y и Z. Такой ящик определяется двумя точками:
//   • min — точка с минимальными координатами (левый-нижний-ближний угол).
//   • max — точка с максимальными координатами (правый-верхний-дальний угол).
//
// AABB используется повсюду:
//   • Для быстрой проверки пересечений (сталкиваются ли два объекта?).
//   • Для отсечения невидимых объектов перед рендерингом (frustum culling).
//   • Для пространственного поиска (какие объекты находятся рядом с точкой?).
//   • Для выделения объектов мышкой (ray casting).
//
// Почему "выровненный по осям"? Потому что стороны ящика всегда параллельны
// осям X, Y, Z. Это делает вычисления очень быстрыми — не нужно считать
// сложные углы поворота. Для повёрнутых объектов AABB немного больше
// самого объекта, но для большинства задач это отличный компромисс
// между точностью и скоростью.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include "../Vector/Vector3.hpp"          // mir::Vector3
#include "../Matrix4.hpp"          // mir::Matrix4 (для transform)
#include <algorithm>                      // std::min, std::max

namespace mir {

class AABB {
public:
    // ── Углы рамки ───────────────────────────────────────────
    Vector3 min;   // минимальные координаты (левый-нижний-ближний угол)
    Vector3 max;   // максимальные координаты (правый-верхний-дальний угол)

    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт пустую (невалидную) рамку.
    // min устанавливается в максимально возможное значение,
    // max — в минимальное, чтобы при первом extend() рамка "схлопнулась"
    // вокруг добавляемой точки.
    constexpr AABB() noexcept
        : min( std::numeric_limits<Scalar>::max(),
               std::numeric_limits<Scalar>::max(),
               std::numeric_limits<Scalar>::max())
        , max(-std::numeric_limits<Scalar>::max(),
              -std::numeric_limits<Scalar>::max(),
              -std::numeric_limits<Scalar>::max())
    {}

    // Создаёт рамку из двух точек.
    constexpr AABB(const Vector3& minPoint, const Vector3& maxPoint) noexcept
        : min(minPoint), max(maxPoint)
    {}

    // ── Геометрические свойства ─────────────────────────────

    // Центр рамки.
    [[nodiscard]] constexpr Vector3 center() const noexcept {
        return {
            (min.x + max.x) * 0.5,
            (min.y + max.y) * 0.5,
            (min.z + max.z) * 0.5
        };
    }

    // Размер рамки по каждой оси (ширина, высота, глубина).
    [[nodiscard]] constexpr Vector3 size() const noexcept {
        return {
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        };
    }

    // ── Расширение рамки ────────────────────────────────────

    // Расширить рамку, чтобы она включала точку.
    void extend(const Vector3& point) noexcept {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    // Расширить рамку, чтобы она включала другую рамку.
    void extend(const AABB& other) noexcept {
        extend(other.min);
        extend(other.max);
    }

    // ── Проверки ─────────────────────────────────────────────

    // Содержит ли рамка точку?
    [[nodiscard]] constexpr bool contains(const Vector3& point) const noexcept {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    // Пересекается ли рамка с другой рамкой?
    [[nodiscard]] constexpr bool intersects(const AABB& other) const noexcept {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    // ── Трансформация ────────────────────────────────────────
    // Применяет матрицу 4×4 к рамке и возвращает НОВУЮ рамку,
    // которая гарантированно содержит все углы исходной после трансформации.
    [[nodiscard]] AABB transformed(const Matrix4& matrix) const noexcept {
        // Берём 8 углов рамки, трансформируем каждый и строим новую рамку.
        Vector3 corners[8] = {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, max.y, min.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        AABB result;
        for (const auto& corner : corners) {
            result.extend(matrix.transformPoint(corner));
        }
        return result;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const AABB& a, const AABB& b) noexcept {
        return a.min == b.min && a.max == b.max;
    }
    friend constexpr bool operator!=(const AABB& a, const AABB& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir