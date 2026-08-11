// MirEngine/Geometry/Solid/Solid.hpp
// 🧊 Базовый класс для всех твёрдых тел (Solid) в MirEngine.
//
// Твёрдое тело — это трёхмерный объект, который имеет объём, границы
// и может быть изготовлен из какого-либо материала. В отличие от
// поверхностей, которые бесконечно тонкие, Solid имеет внутренность
// и внешнюю оболочку.
//
// Этот абстрактный класс определяет общий интерфейс для всех твёрдых тел:
//   • volume()        — объём тела.
//   • surfaceArea()   — площадь поверхности.
//   • contains(point) — проверка, находится ли точка внутри тела.
//   • closestPoint()  — ближайшая точка на поверхности к заданной.
//   • intersect(ray)  — пересечение с лучом (трассировка лучей).
//   • boundingBox()   — выровненная по осям ограничивающая рамка (AABB).
//   • transform()     — создать новое тело, преобразованное матрицей.
//   • tessellate()    — создать сетку треугольников для визуализации.
//
// Конкретные твёрдые тела (Box, Cylinder, Sphere, Cone, Torus...)
// наследуются от этого класса и реализуют методы для своей формы.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Point/Point3.hpp"          // mir::Point3
#include "../Direction/Direction3.hpp"   // mir::Direction3
#include "../../Math/Vector/Vector3.hpp" // mir::Vector3
#include "../../Math/Bounds/AABB.hpp"    // mir::AABB
#include "../../Math/Transform.hpp"      // mir::Transform
#include "../Ray/Ray3.hpp"              // mir::Ray3
#include "../../Core/Types/Scalar.hpp"   // mir::Scalar
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <optional>

namespace mir {

class Solid {
public:
    virtual ~Solid() = default;

    // ── Основные геометрические свойства ────────────────────

    [[nodiscard]] virtual Scalar volume() const = 0;
    [[nodiscard]] virtual Scalar surfaceArea() const = 0;

    // ── Проверка принадлежности точки ───────────────────────

    [[nodiscard]] virtual bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const = 0;

    // ── Ближайшая точка на поверхности ──────────────────────

    [[nodiscard]] virtual Point3 closestPoint(const Point3& point) const = 0;

    // ── Пересечение с лучом ─────────────────────────────────

    [[nodiscard]] virtual std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const = 0;

    // ── Ограничивающая рамка (AABB) ─────────────────────────

    [[nodiscard]] virtual AABB boundingBox() const = 0;

    // ── Трансформация ───────────────────────────────────────

    [[nodiscard]] virtual std::unique_ptr<Solid> transformed(const Transform& transform) const {
        return nullptr; // базовая реализация-заглушка
    }

    // ── Тесселяция (создание сетки треугольников) ───────────

    struct Mesh {
        std::vector<Point3> vertices;
        std::vector<std::array<int, 3>> faces;
    };

    [[nodiscard]] virtual Mesh tessellate(int subdivisions = 16) const = 0;

    // ── Тип тела ────────────────────────────────────────────

    [[nodiscard]] virtual std::string typeName() const = 0;
};

} // namespace mir