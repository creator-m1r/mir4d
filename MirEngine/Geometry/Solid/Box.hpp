// MirEngine/Geometry/Solid/Box.hpp
// 📦 Прямоугольный параллелепипед (Box) — твёрдое тело с шестью гранями.
//
// Box — это самый простой трёхмерный примитив. Он задаётся центром,
// размерами (ширина, высота, глубина) и ориентацией (вращение).
// В отличие от AABB, который всегда выровнен по осям координат,
// Box может быть повёрнут под любым углом благодаря встроенному Transform.
//
// Box используется для:
//   • Быстрого прототипирования ("поставить кубик").
//   • Задания рабочих зон и ограничений.
//   • Упрощённого представления сложных объектов (bounding box).
//   • Булевых операций (объединение, вычитание, пересечение).
//
// Особенности реализации:
//   • Размеры всегда положительные (по модулю).
//   • Центр — геометрический центр параллелепипеда.
//   • Transform позволяет вращать и перемещать коробку.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Solid.hpp"                    // Базовый класс Solid
#include "../Point/Point3.hpp"          // mir::Point3
#include "../Direction/Direction3.hpp"   // mir::Direction3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Math/Bounds/AABB.hpp"    // mir::AABB
#include "../../Math/Transform.hpp"      // mir::Transform
#include "../Ray/Ray3.hpp"              // mir::Ray3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>

namespace mir {

class Box : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт параллелепипед с центром в начале координат и заданными размерами.
    // Размеры должны быть положительными.
    constexpr Box(Scalar width, Scalar height, Scalar depth) noexcept
        : m_center(Point3::origin())
        , m_halfExtents(std::abs(width) * Scalar(0.5),
                        std::abs(height) * Scalar(0.5),
                        std::abs(depth) * Scalar(0.5))
        , m_transform(Transform::identity())
    {}

    // Создаёт параллелепипед с центром, размерами и поворотом.
    Box(const Point3& center, Scalar width, Scalar height, Scalar depth,
        const Quaternion& rotation = Quaternion::identity()) noexcept
        : m_center(center)
        , m_halfExtents(std::abs(width) * Scalar(0.5),
                        std::abs(height) * Scalar(0.5),
                        std::abs(depth) * Scalar(0.5))
        , m_transform(center, rotation, {1.0, 1.0, 1.0})
    {}

    // Создаёт параллелепипед по полному Transform.
    explicit Box(const Transform& transform, const Vector3& halfExtents) noexcept
        : m_center(transform.position)
        , m_halfExtents(halfExtents.x, halfExtents.y, halfExtents.z)
        , m_transform(transform)
    {}

    // ── Доступ к параметрам ─────────────────────────────────

    [[nodiscard]] constexpr Point3 center()      const noexcept { return m_center; }
    [[nodiscard]] constexpr Vector3 halfExtents() const noexcept { return m_halfExtents; }
    [[nodiscard]] constexpr Scalar width()       const noexcept { return m_halfExtents.x * Scalar(2); }
    [[nodiscard]] constexpr Scalar height()      const noexcept { return m_halfExtents.y * Scalar(2); }
    [[nodiscard]] constexpr Scalar depth()       const noexcept { return m_halfExtents.z * Scalar(2); }
    [[nodiscard]] const Transform& transform()   const noexcept { return m_transform; }

    // ── Реализация Solid ────────────────────────────────────

    // Объём параллелепипеда = ширина × высота × глубина.
    [[nodiscard]] Scalar volume() const override {
        return m_halfExtents.x * m_halfExtents.y * m_halfExtents.z * Scalar(8);
    }

    // Площадь поверхности = 2 × (ширина×высота + ширина×глубина + высота×глубина).
    [[nodiscard]] Scalar surfaceArea() const override {
        Scalar w = width();
        Scalar h = height();
        Scalar d = depth();
        return Scalar(2) * (w * h + w * d + h * d);
    }

    // Проверка, находится ли точка внутри параллелепипеда.
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        // Преобразуем точку в локальную систему координат параллелепипеда.
        Transform inv = m_transform.inverse();
        Point3 local = inv.transformPoint(point);
        return std::abs(local.x) <= m_halfExtents.x + tolerance &&
               std::abs(local.y) <= m_halfExtents.y + tolerance &&
               std::abs(local.z) <= m_halfExtents.z + tolerance;
    }

    // Ближайшая точка на поверхности параллелепипеда.
    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        // Преобразуем точку в локальную систему координат.
        Transform inv = m_transform.inverse();
        Point3 local = inv.transformPoint(point);

        // Ближайшая точка на AABB в локальной системе.
        Point3 closestLocal{
            std::clamp(local.x, -m_halfExtents.x, m_halfExtents.x),
            std::clamp(local.y, -m_halfExtents.y, m_halfExtents.y),
            std::clamp(local.z, -m_halfExtents.z, m_halfExtents.z)
        };

        // Если точка уже внутри, нужно "вытолкнуть" её на ближайшую грань.
        if (closestLocal == local) {
            // Находим ближайшую грань.
            Scalar distToXMin = std::abs(local.x + m_halfExtents.x);
            Scalar distToXMax = std::abs(local.x - m_halfExtents.x);
            Scalar distToYMin = std::abs(local.y + m_halfExtents.y);
            Scalar distToYMax = std::abs(local.y - m_halfExtents.y);
            Scalar distToZMin = std::abs(local.z + m_halfExtents.z);
            Scalar distToZMax = std::abs(local.z - m_halfExtents.z);

            Scalar minDist = distToXMin;
            Point3 pushed = { -m_halfExtents.x, local.y, local.z };

            if (distToXMax < minDist) { minDist = distToXMax; pushed = { m_halfExtents.x, local.y, local.z }; }
            if (distToYMin < minDist) { minDist = distToYMin; pushed = { local.x, -m_halfExtents.y, local.z }; }
            if (distToYMax < minDist) { minDist = distToYMax; pushed = { local.x,  m_halfExtents.y, local.z }; }
            if (distToZMin < minDist) { minDist = distToZMin; pushed = { local.x, local.y, -m_halfExtents.z }; }
            if (distToZMax < minDist) { minDist = distToZMax; pushed = { local.x, local.y,  m_halfExtents.z }; }

            closestLocal = pushed;
        }

        // Преобразуем обратно в мировые координаты.
        return m_transform.transformPoint(closestLocal);
    }

    // Пересечение с лучом.
    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        // Преобразуем луч в локальную систему координат.
        Transform inv = m_transform.inverse();
        Point3 localOrigin = inv.transformPoint(ray.origin);
        Vector3 localDir = inv.transformDirection(ray.direction);

        // Пересечение с AABB в локальной системе (алгоритм slabs).
        Scalar tMin = -std::numeric_limits<Scalar>::infinity();
        Scalar tMax =  std::numeric_limits<Scalar>::infinity();

        for (int i = 0; i < 3; ++i) {
            Scalar halfExt = (&m_halfExtents.x)[i];
            Scalar origin = (&localOrigin.x)[i];
            Scalar dir = (&localDir.x)[i];

            if (std::abs(dir) < tolerance) {
                // Луч параллелен этой оси.
                if (origin < -halfExt || origin > halfExt) {
                    return std::nullopt;   // не пересекает
                }
                continue;
            }

            Scalar t1 = (-halfExt - origin) / dir;
            Scalar t2 = ( halfExt - origin) / dir;
            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) return std::nullopt;
        }

        if (tMin < tolerance) {
            tMin = tMax;   // луч начинается внутри или очень близко
        }

        if (tMin < tolerance) return std::nullopt;

        // Точка пересечения в локальной системе.
        Point3 localHit = localOrigin + localDir * tMin;
        // Преобразуем обратно в мировые координаты.
        return m_transform.transformPoint(localHit);
    }

    // Ограничивающая рамка (AABB) в мировых координатах.
    [[nodiscard]] AABB boundingBox() const override {
        // 8 углов параллелепипеда в локальной системе.
        std::array<Point3, 8> corners = {{
            {-m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z},
            { m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z},
            {-m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z},
            {-m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z},
            { m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z},
            { m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z},
            {-m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z},
            { m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z}
        }};

        AABB aabb;
        for (const auto& corner : corners) {
            // Временное преобразование Point3 в Vector3 для AABB::extend
            // (будет исправлено после перехода AABB на Point3)
            aabb.extend(Vector3(corner.x, corner.y, corner.z));
        }
        return aabb;
    }

    // Трансформация — создаём новый Box с обновлённым Transform.
    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newBox = std::make_unique<Box>(*this);
        newBox->m_transform = t.combine(m_transform);
        newBox->m_center = newBox->m_transform.position;
        return newBox;
    }

    // Тесселяция (создание сетки треугольников).
    [[nodiscard]] Solid::Mesh tessellate(int /*subdivisions*/ = 1) const override {
        // 8 вершин параллелепипеда в локальной системе.
        std::array<Point3, 8> localVertices = {{
            {-m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z},  // 0
            { m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z},  // 1
            { m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z},  // 2
            {-m_halfExtents.x,  m_halfExtents.y, -m_halfExtents.z},  // 3
            {-m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z},  // 4
            { m_halfExtents.x, -m_halfExtents.y,  m_halfExtents.z},  // 5
            { m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z},  // 6
            {-m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z}   // 7
        }};

        // Преобразуем в мировые координаты.
        Mesh mesh;
        mesh.vertices.reserve(8);
        for (const auto& v : localVertices) {
            mesh.vertices.push_back(m_transform.transformPoint(v));
        }

        // 12 треугольников (по 2 на каждую грань).
        mesh.faces = {
            // Передняя грань (z-)
            {0, 1, 2}, {0, 2, 3},
            // Задняя грань (z+)
            {4, 6, 5}, {4, 7, 6},
            // Левая грань (x-)
            {0, 3, 7}, {0, 7, 4},
            // Правая грань (x+)
            {1, 5, 6}, {1, 6, 2},
            // Нижняя грань (y-)
            {0, 4, 5}, {0, 5, 1},
            // Верхняя грань (y+)
            {3, 2, 6}, {3, 6, 7}
        };

        return mesh;
    }

    // Имя типа.
    [[nodiscard]] std::string typeName() const override {
        return "Box";
    }

private:
    Point3   m_center;         // геометрический центр
    Vector3  m_halfExtents;    // половинные размеры (ширина/2, высота/2, глубина/2)
    Transform m_transform;     // позиция + вращение (масштаб уже учтён в halfExtents)
};

} // namespace mir