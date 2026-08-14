// MirEngine/Geometry/Solid/Wedge.hpp
// 🔻 Клин (Wedge) — твёрдое тело в виде треугольной призмы.
//
// Клин (Wedge) — это тело, которое получается, если взять прямоугольный
// параллелепипед и разрезать его диагональной плоскостью. У него есть
// прямоугольное основание (ширина × глубина), одна вертикальная грань
// (высота h1) и одна наклонная грань (высота убывает от h1 до h2).
// Если h1 = h2, клин превращается в обычный параллелепипед.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Solid.hpp"
#include "../Point/Point3.hpp"
#include "../Direction/Direction3.hpp"
#include "../../Math/Vector/Vector3.hpp"
#include "../../Math/Bounds/AABB.hpp"
#include "../../Math/Transform.hpp"
#include "../Ray/Ray3.hpp"
#include "../Plane/Plane.hpp"
#include <cmath>
#include <array>
#include <memory>
#include <algorithm>
#include <limits>

namespace mir {

class Wedge : public Solid {
public:
    // ── Конструкторы ─────────────────────────────────────────
    Wedge(const Point3& baseCenter,
          const Direction3& dirX, Scalar width,
          const Direction3& dirY, Scalar depth,
          const Direction3& dirZ,
          Scalar h1, Scalar h2) noexcept
        : m_baseCenter(baseCenter)
        , m_dirX(dirX)
        , m_width(std::abs(width))
        , m_dirY(dirY)
        , m_depth(std::abs(depth))
        , m_dirZ(dirZ)
        , m_h1(std::abs(h1))
        , m_h2(std::abs(h2))
    {}

    Wedge(const Point3& baseCenter, Scalar width, Scalar depth,
          Scalar h1, Scalar h2) noexcept
        : Wedge(baseCenter, Direction3::unitX(), width,
                Direction3::unitY(), depth,
                Direction3::unitZ(), h1, h2)
    {}

    // ── Доступ к параметрам ─────────────────────────────────
    [[nodiscard]] Point3     baseCenter() const noexcept { return m_baseCenter; }
    [[nodiscard]] Direction3 dirX()       const noexcept { return m_dirX; }
    [[nodiscard]] Scalar     width()      const noexcept { return m_width; }
    [[nodiscard]] Direction3 dirY()       const noexcept { return m_dirY; }
    [[nodiscard]] Scalar     depth()      const noexcept { return m_depth; }
    [[nodiscard]] Direction3 dirZ()       const noexcept { return m_dirZ; }
    [[nodiscard]] Scalar     h1()         const noexcept { return m_h1; }
    [[nodiscard]] Scalar     h2()         const noexcept { return m_h2; }

    // Получить все 8 вершин клина
    [[nodiscard]] std::array<Point3, 8> vertices() const noexcept {
        Scalar halfW = m_width * Scalar(0.5);
        Scalar halfD = m_depth * Scalar(0.5);
        Vector3 dX = m_dirX.asVector() * halfW;
        Vector3 dY = m_dirY.asVector() * halfD;
        Vector3 dZ1 = m_dirZ.asVector() * m_h1;
        Vector3 dZ2 = m_dirZ.asVector() * m_h2;

        return {
            m_baseCenter - dX - dY,                   // 0: нижняя передняя левая
            m_baseCenter + dX - dY,                   // 1: нижняя передняя правая
            m_baseCenter + dX + dY,                   // 2: нижняя задняя правая
            m_baseCenter - dX + dY,                   // 3: нижняя задняя левая
            m_baseCenter - dX - dY + dZ1,             // 4: верхняя передняя левая (h1)
            m_baseCenter + dX - dY + dZ1,             // 5: верхняя передняя правая (h1)
            m_baseCenter + dX + dY + dZ2,             // 6: верхняя задняя правая (h2)
            m_baseCenter - dX + dY + dZ2              // 7: верхняя задняя левая (h2)
        };
    }

    // ── Реализация Solid ────────────────────────────────────

    [[nodiscard]] Scalar volume() const override {
        return Scalar(0.5) * m_width * m_depth * (m_h1 + m_h2);
    }

    [[nodiscard]] Scalar surfaceArea() const override {
        auto verts = vertices();
        // Площадь основания
        Scalar baseArea = m_width * m_depth;

        // Верхняя грань – наклонный прямоугольник (или две треугольные, если h1!=h2)
        Vector3 diag = verts[6] - verts[4]; // (h2-h1) по высоте + глубине
        Scalar topArea = (std::abs(m_h1 - m_h2) < Scalar(1e-10))
            ? m_width * m_depth
            : m_width * diag.length();

        // Передняя грань (прямоугольник высотой h1)
        Scalar frontArea = m_width * m_h1;
        // Задняя грань (прямоугольник высотой h2)
        Scalar backArea = m_width * m_h2;
        // Левая грань – трапеция
        Vector3 leftEdge = verts[7] - verts[3]; // (h2) по высоте + 0 по глубине? Нужно вычислить.
        // Левая грань: вершины 0,3,7,4. Считаем как сумму двух треугольников.
        Scalar leftArea = Scalar(0.5) * (m_depth * m_h2) + Scalar(0.5) * (m_depth * m_h1);
        // Правая грань – аналогично
        Scalar rightArea = Scalar(0.5) * (m_depth * m_h2) + Scalar(0.5) * (m_depth * m_h1);

        return baseArea + topArea + frontArea + backArea + leftArea + rightArea;
    }

    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = Scalar(1e-10)) const override {
        Vector3 toPoint = point - m_baseCenter;
        Scalar localZ = Vector3::dot(toPoint, m_dirZ.asVector());
        if (localZ < -tolerance || localZ > std::max(m_h1, m_h2) + tolerance) return false;

        Scalar localX = Vector3::dot(toPoint, m_dirX.asVector());
        Scalar halfW = m_width * Scalar(0.5);
        if (localX < -halfW - tolerance || localX > halfW + tolerance) return false;

        Scalar localY = Vector3::dot(toPoint, m_dirY.asVector());
        Scalar halfD = m_depth * Scalar(0.5);
        if (localY < -halfD - tolerance || localY > halfD + tolerance) return false;

        Scalar t = (localY + halfD) / m_depth;   // 0 на фронте (h1), 1 на тылу (h2)
        Scalar heightHere = m_h1 + (m_h2 - m_h1) * t;
        return localZ <= heightHere + tolerance;
    }

    [[nodiscard]] Point3 closestPoint(const Point3& point) const override {
        auto verts = vertices();
        Point3 bestPt = verts[0];
        Scalar bestDist = Point3::distance(point, bestPt);
        for (int i = 1; i < 8; ++i) {
            Scalar d = Point3::distance(point, verts[i]);
            if (d < bestDist) { bestDist = d; bestPt = verts[i]; }
        }
        return bestPt;
    }

    [[nodiscard]] std::optional<Point3> intersect(const Ray3& ray, Scalar tolerance = Scalar(1e-10)) const override {
        auto verts = vertices();
        // Строим 6 плоскостей граней
        std::array<Plane, 6> faces = {{
            // Нижняя грань (0,1,2,3) – нормаль -dirZ
            Plane(m_baseCenter, Direction3::fromVector(-m_dirZ.asVector())),
            // Передняя грань (0,1,5,4)
            Plane(verts[0], Direction3::fromVector(-m_dirY.asVector())),
            // Задняя грань (2,3,7,6)
            Plane(verts[2], Direction3::fromVector(m_dirY.asVector())),
            // Левая грань (0,3,7,4)
            Plane(verts[0], Direction3::fromVector(-m_dirX.asVector())),
            // Правая грань (1,2,6,5)
            Plane(verts[1], Direction3::fromVector(m_dirX.asVector())),
            // Верхняя грань (4,5,6,7) – наклонная. Нормаль через cross product
            Plane(verts[4], Direction3::fromVector(
                Vector3::cross(verts[5] - verts[4], verts[7] - verts[4]).normalized()))
        }};

        std::optional<Point3> bestHit;
        Scalar bestT = std::numeric_limits<Scalar>::max();

        for (const auto& face : faces) {
            auto optHit = face.intersect(ray);
            if (!optHit) continue;
            Scalar t = Vector3::dot(*optHit - ray.origin, ray.direction) / ray.direction.lengthSquared();
            if (t > tolerance && t < bestT && contains(*optHit, tolerance)) {
                bestHit = optHit;
                bestT = t;
            }
        }
        return bestHit;
    }

    [[nodiscard]] AABB boundingBox() const override {
        AABB aabb;
        auto extendPoint = [&aabb](const Point3& p) {
            aabb.extend(Vector3(p.x, p.y, p.z));
        };

        Scalar halfW = m_width * Scalar(0.5);
        Scalar halfD = m_depth * Scalar(0.5);
        Vector3 dX = m_dirX.asVector() * halfW;
        Vector3 dY = m_dirY.asVector() * halfD;

        for (Scalar sx = -Scalar(1); sx <= Scalar(1); sx += Scalar(2)) {
            for (Scalar sy = -Scalar(1); sy <= Scalar(1); sy += Scalar(2)) {
                extendPoint(m_baseCenter + dX * sx + dY * sy);
                Scalar h = (sy < Scalar(0)) ? m_h1 : m_h2;
                extendPoint(m_baseCenter + dX * sx + dY * sy + m_dirZ.asVector() * h);
            }
        }
        return aabb;
    }

    [[nodiscard]] std::unique_ptr<Solid> transformed(const Transform& t) const override {
        auto newWedge = std::make_unique<Wedge>(*this);
        newWedge->m_baseCenter = t.transformPoint(m_baseCenter);
        newWedge->m_dirX = Direction3::fromVector(t.transformDirection(m_dirX.asVector()));
        newWedge->m_dirY = Direction3::fromVector(t.transformDirection(m_dirY.asVector()));
        newWedge->m_dirZ = Direction3::fromVector(t.transformDirection(m_dirZ.asVector()));
        return newWedge;
    }

    [[nodiscard]] Solid::Mesh tessellate(int /*subdivisions*/ = 1) const override {
        Mesh mesh;
        auto verts = vertices();
        mesh.vertices.assign(verts.begin(), verts.end());

        mesh.faces = {
            {0, 1, 5}, {0, 5, 4},  // передняя
            {2, 3, 7}, {2, 7, 6},  // задняя
            {0, 3, 2}, {0, 2, 1},  // нижняя
            {4, 5, 6}, {4, 6, 7},  // верхняя
            {0, 4, 7}, {0, 7, 3},  // левая
            {1, 2, 6}, {1, 6, 5}   // правая
        };
        return mesh;
    }

    [[nodiscard]] std::string typeName() const override {
        return "Wedge";
    }

private:
    Point3      m_baseCenter;
    Direction3  m_dirX;
    Scalar      m_width;
    Direction3  m_dirY;
    Scalar      m_depth;
    Direction3  m_dirZ;
    Scalar      m_h1;
    Scalar      m_h2;
};

} // namespace mir