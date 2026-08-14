// MirEngine/Geometry/Solid/Sphere.hpp
// ⚪ Сфера — твёрдое тело, все точки поверхности которого равноудалены от центра.
//
// Сфера — это самый симметричный трёхмерный примитив. Она задаётся центром
// и радиусом. В отличие от шара (который включает внутренность), мы называем
// её Solid, потому что она реализует интерфейс твёрдого тела.
//
// Сфера используется для:
//   • Моделирования шариков, сферических соединений, линз.
//   • Упрощённого представления объектов в физических расчётах.
//   • Построения сложных тел через булевы операции.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Solid.hpp"
#include "../Point/Point3.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../../Math/Bounds/AABB.hpp"
#include "../../Math/Transform.hpp"
#include "../Ray/Ray3.hpp"
#include <cmath>
#include <array>
#include <memory>

namespace mir {

class Sphere : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────

    // Создаёт сферу с центром в начале координат и заданным радиусом.
    explicit Sphere(Scalar radius = Scalar(1)) noexcept
        : m_center(Point3::origin())
        , m_radius(std::abs(radius))
    {}

    // Создаёт сферу с заданным центром и радиусом.
    Sphere(const Point3& center, Scalar radius) noexcept
        : m_center(center)
        , m_radius(std::abs(radius))
    {}

    // ── Доступ к параметрам ─────────────────────────────────
    [[nodiscard]] Point3 center() const noexcept { return m_center; }
    [[nodiscard]] Scalar radius() const noexcept { return m_radius; }
    [[nodiscard]] Scalar diameter() const noexcept { return m_radius * Scalar(2); }

    // ── Реализация Solid ────────────────────────────────────

    // Объём сферы = 4/3 × π × r³
    [[nodiscard]] Scalar volume() const override {
        return (Scalar(4) / Scalar(3)) * Scalar(3.14159265358979323846)
             * m_radius * m_radius * m_radius;
    }

    // Площадь поверхности = 4 × π × r²
    [[nodiscard]] Scalar surfaceArea() const override {
        return Scalar(4) * Scalar(3.14159265358979323846) * m_radius * m_radius;
    }

    // Проверка, находится ли точка внутри сферы (включая границу).
    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        return Point3::distanceSquared(point, m_center) <= 
               (m_radius + tolerance) * (m_radius + tolerance);
    }

    // Ближайшая точка на поверхности сферы к заданной точке.
    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        Vector3 toPoint = point - m_center;
        Scalar dist = toPoint.length();

        // Если точка в центре — любая точка на поверхности (возвращаем на оси X).
        if (dist < Scalar(1e-20)) {
            return m_center + Vector3::unitX() * m_radius;
        }

        // Иначе — нормализуем вектор и умножаем на радиус.
        return m_center + (toPoint / dist) * m_radius;
    }

    // Пересечение с лучом (аналитическое решение через квадратное уравнение).
    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 ro = ray.origin - m_center;   // вектор от центра сферы к началу луча
        Vector3 rd = ray.direction;           // направление луча

        // Квадратное уравнение: |ro + t*rd|² = r²
        Scalar a = Vector3::dot(rd, rd);
        Scalar b = Scalar(2) * Vector3::dot(rd, ro);
        Scalar c = Vector3::dot(ro, ro) - m_radius * m_radius;

        Scalar discriminant = b * b - Scalar(4) * a * c;
        if (discriminant < Scalar(0)) return std::nullopt;   // нет пересечения

        Scalar sqrtDisc = std::sqrt(discriminant);
        Scalar t1 = (-b - sqrtDisc) / (Scalar(2) * a);
        Scalar t2 = (-b + sqrtDisc) / (Scalar(2) * a);

        // Берём ближайшее положительное t.
        Scalar t = (t1 > tolerance) ? t1 : (t2 > tolerance) ? t2 : -Scalar(1);
        if (t < tolerance) return std::nullopt;

        // Явное конструирование Point3 (избегаем проблем с ray.pointAt, возвращающим Vector3)
        return Point3(
            ray.origin.x + ray.direction.x * t,
            ray.origin.y + ray.direction.y * t,
            ray.origin.z + ray.direction.z * t
        );
    }

    // Ограничивающая рамка (AABB) — куб со стороной 2*radius.
    [[nodiscard]] AABB boundingBox() const override {
        AABB aabb;
        // Временная лямбда для расширения AABB точками (конвертируем Point3 -> Vector3)
        auto extendPoint = [&aabb](const Point3& p) {
            aabb.extend(Vector3(p.x, p.y, p.z));
        };

        Point3 minPt(m_center.x - m_radius, m_center.y - m_radius, m_center.z - m_radius);
        Point3 maxPt(m_center.x + m_radius, m_center.y + m_radius, m_center.z + m_radius);

        extendPoint(minPt);
        extendPoint(maxPt);

        return aabb;
    }

    // Трансформация сферы. Если масштаб неравномерный, сфера превращается в эллипсоид.
    // Здесь мы упрощаем: применяем Transform к центру и масштабируем радиус по максимальной оси.
    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newSphere = std::make_unique<Sphere>(*this);
        newSphere->m_center = t.transformPoint(m_center);

        // Если масштаб неравномерный, сфера становится эллипсоидом.
        // Для простоты используем максимальный масштаб как новый радиус.
        Scalar maxScale = std::max({t.scale.x, t.scale.y, t.scale.z});
        newSphere->m_radius = m_radius * std::abs(maxScale);

        return newSphere;
    }

    // Тесселяция — создаём UV-сферу.
    [[nodiscard]] Solid::Mesh tessellate(int subdivisions = 16) const override {
        Mesh mesh;
        int stacks = std::max(2, subdivisions / 2);     // число параллелей
        int slices = std::max(3, subdivisions);          // число меридианов

        // Вершины (полюса + кольца).
        mesh.vertices.push_back(m_center + Vector3::unitY() * m_radius);   // верхний полюс (0)

        for (int i = 1; i < stacks; ++i) {
            Scalar phi = Scalar(3.14159265358979323846) * Scalar(i) / Scalar(stacks);
            Scalar cosPhi = std::cos(phi);
            Scalar sinPhi = std::sin(phi);

            for (int j = 0; j < slices; ++j) {
                Scalar theta = Scalar(2) * Scalar(3.14159265358979323846) * Scalar(j) / Scalar(slices);
                Scalar cosTheta = std::cos(theta);
                Scalar sinTheta = std::sin(theta);

                Vector3 offset{
                    sinPhi * cosTheta,
                    cosPhi,
                    sinPhi * sinTheta
                };
                mesh.vertices.push_back(m_center + offset * m_radius);
            }
        }

        mesh.vertices.push_back(m_center - Vector3::unitY() * m_radius);   // нижний полюс (последний)

        // Треугольники.
        // Верхние треугольники (от верхнего полюса к первому кольцу).
        for (int j = 0; j < slices; ++j) {
            int nextJ = (j + 1) % slices;
            mesh.faces.push_back({0, 1 + j, 1 + nextJ});
        }

        // Средние кольца.
        for (int i = 0; i < stacks - 2; ++i) {
            for (int j = 0; j < slices; ++j) {
                int nextJ = (j + 1) % slices;
                int curr = 1 + i * slices + j;
                int next = 1 + i * slices + nextJ;
                int currLower = 1 + (i + 1) * slices + j;
                int nextLower = 1 + (i + 1) * slices + nextJ;

                mesh.faces.push_back({curr, next, nextLower});
                mesh.faces.push_back({curr, nextLower, currLower});
            }
        }

        // Нижние треугольники (от нижнего полюса к последнему кольцу).
        int lastRingStart = 1 + (stacks - 2) * slices;
        int southPole = static_cast<int>(mesh.vertices.size()) - 1;
        for (int j = 0; j < slices; ++j) {
            int nextJ = (j + 1) % slices;
            mesh.faces.push_back({southPole, lastRingStart + nextJ, lastRingStart + j});
        }

        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Sphere";
    }

private:
    Point3 m_center;   // центр сферы
    Scalar m_radius;   // радиус (положительное число)
};

} // namespace mir