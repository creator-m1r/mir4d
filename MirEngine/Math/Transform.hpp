// MirEngine/Math/Transform/Transform.hpp
// 🧩 Трансформация — объединяет позицию, вращение и масштаб в одном объекте.
//
// В 3D-графике и CAD каждый объект имеет три свойства:
//   1. Где он находится?           → position (Point3)
//   2. Как он повёрнут?            → rotation (Quaternion)
//   3. Насколько он растянут/сжат? → scale (Vector3)
//
// Transform собирает все три свойства в одну удобную структуру.
// С его помощью можно:
//   • Перемещать объект в пространстве.
//   • Вращать объект вокруг любой оси.
//   • Масштабировать объект (равномерно или по осям).
//   • Комбинировать несколько трансформаций (например, родитель + потомок).
//   • Преобразовывать точки и направления из локальной системы координат
//     в мировую и обратно.
//
// Transform — это как "паспорт" объекта в 3D-мире. Зная Transform,
// можно точно сказать, где объект находится и как ориентирован.
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include "../../Core/Types/Scalar.hpp"        // mir::Scalar = double
#include "../Vector/Vector3.hpp"              // mir::Vector3
#include "../Quaternion/Quaternion.hpp"       // mir::Quaternion
#include "../Matrix4.hpp"                     // mir::Matrix4
#include "Geometry/Point/Point3.hpp"    // mir::Point3

namespace mir {

class Transform {
public:
    // ── Компоненты ───────────────────────────────────────────
    Point3     position = Point3{};                  // где находится объект (x, y, z)
    Quaternion rotation = Quaternion::identity();    // как повёрнут
    Vector3    scale    = {1.0, 1.0, 1.0};          // масштаб (1.0 = исходный размер)

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Transform() noexcept = default;

    constexpr Transform(const Point3& pos, const Quaternion& rot = Quaternion::identity(),
                        const Vector3& scl = {1.0, 1.0, 1.0}) noexcept
        : position(pos), rotation(rot), scale(scl)
    {}

    // ── Статические фабрики ──────────────────────────────────
    [[nodiscard]] static constexpr Transform identity() noexcept {
        return Transform{};
    }

    // ── Преобразование в матрицу 4×4 ────────────────────────
    // Строит матрицу model-to-world: сначала масштаб, потом вращение, потом перенос.
    [[nodiscard]] Matrix4 matrix() const noexcept {
        Matrix4 s = Matrix4::scale(scale);
        Matrix4 r = rotation.toMatrix4();
        // Временное преобразование Point3 → Vector3 для Matrix4::translation
        // (будет исправлено после обновления Matrix4 на Point3)
        Vector3 posVec(position.x, position.y, position.z);
        Matrix4 t = Matrix4::translation(posVec);
        return t * r * s;
    }

    // ── Применение трансформации к точке ────────────────────
    // Преобразует точку из локальной системы координат в мировую.
    [[nodiscard]] Point3 transformPoint(const Point3& localPoint) const noexcept {
        // Вычисляем через матрицу, временно конвертируя Point3 ↔ Vector3
        Vector3 lp(localPoint.x, localPoint.y, localPoint.z);
        Vector3 result = matrix().transformPoint(lp);
        return Point3(result.x, result.y, result.z);
    }

    // ── Применение трансформации к направлению ──────────────
    // Направления не переносятся, только вращаются и масштабируются.
    [[nodiscard]] Vector3 transformDirection(const Vector3& localDir) const noexcept {
        return matrix().transformDirection(localDir);
    }

    // ── Комбинирование трансформаций ────────────────────────
    // Сначала применяется this, потом other.
    [[nodiscard]] Transform combine(const Transform& other) const noexcept {
        Transform result;
        result.scale    = Vector3{scale.x * other.scale.x, scale.y * other.scale.y, scale.z * other.scale.z};
        result.rotation = rotation * other.rotation;
        result.position = transformPoint(other.position);   // Point3 = Point3
        return result;
    }

    // ── Обратная трансформация ──────────────────────────────
    [[nodiscard]] Transform inverse() const noexcept {
        Transform result;
        result.scale    = {1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z};
        result.rotation = rotation.inverse();

        // Обратный перенос: смещение = -position (вектор из точки в начало координат)
        Vector3 negPos = Vector3{} - Vector3(position.x, position.y, position.z);
        // Поворачиваем это смещение и масштабируем
        Vector3 transformedNegPos = result.rotation.rotate(negPos);
        transformedNegPos.x /= scale.x;
        transformedNegPos.y /= scale.y;
        transformedNegPos.z /= scale.z;
        result.position = Point3(transformedNegPos.x, transformedNegPos.y, transformedNegPos.z);
        return result;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Transform& a, const Transform& b) noexcept {
        return a.position == b.position && a.rotation == b.rotation && a.scale == b.scale;
    }
    friend constexpr bool operator!=(const Transform& a, const Transform& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir