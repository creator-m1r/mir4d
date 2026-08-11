// MirEngine/Scene/Camera.h
// =================================================================================
// Камера сцены.
//
// Определяет точку обзора и параметры проекции. Генерирует view и projection
// матрицы, используемые RenderContext. Поддерживает перспективную и ортогональную
// проекции, а также орбитальное вращение вокруг целевой точки (target).
//
// Архитектура:
//   - Не зависит от рендеринг-бэкенда, только математика (Vector3, Matrix4).
//   - Используется Viewport для обновления RenderContext перед кадром.
//   - Может управляться контроллером камеры (будущий CameraController).
//
// Типы проекций:
//   - Perspective: классическая перспектива (fov, aspect, near/far).
//   - Orthographic: ортогональная (left, right, bottom, top, near/far) —
//     будет полезна для 2D-чертежей и некоторых CAD-режимов.
//
// Орбита:
//   - Позиция камеры вычисляется сферически относительно target.
//   - Углы theta (азимут), phi (зенит) и радиус distance определяют позицию.
//   - Направление взгляда всегда на target.
// =================================================================================

// MirEngine/Scene/Camera.h
#pragma once

#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace MirEngine {

struct Vector3 {
    float x = 0.f, y = 0.f, z = 0.f;
    constexpr Vector3() = default;
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3 operator+(const Vector3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vector3 operator*(float s) const { return {x*s, y*s, z*s}; }
};

using Matrix4Raw = std::array<float, 16>; // column-major

enum class ProjectionType { Perspective, Orthographic };

class Camera {
public:
    Camera();

    // --- Проекция ---
    void setPerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setAspect(float aspect);

    // --- Орбита ---
    void setTarget(const Vector3& target);
    void setOrbit(float theta, float phi, float distance);
    void setDistance(float distance);
    void setTheta(float theta);
    void setPhi(float phi);

    [[nodiscard]] Vector3 getTarget()   const noexcept { return m_target; }
    [[nodiscard]] float   getDistance() const noexcept { return m_distance; }
    [[nodiscard]] float   getTheta()    const noexcept { return m_theta; }
    [[nodiscard]] float   getPhi()      const noexcept { return m_phi; }
    [[nodiscard]] Vector3 getPosition() const;

    // --- Матрицы (ленивый пересчёт) ---
    [[nodiscard]] Matrix4Raw getViewMatrix() const;
    [[nodiscard]] Matrix4Raw getProjectionMatrix() const;

    [[nodiscard]] float getNearPlane() const noexcept { return m_near; }
    [[nodiscard]] float getFarPlane()  const noexcept { return m_far; }

    // Луч из экранных координат (для picking)
    void getRayFromScreen(float screenX, float screenY,
                          uint32_t vpWidth, uint32_t vpHeight,
                          Vector3& outOrigin, Vector3& outDirection) const;

private:
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;
    void updatePositionFromOrbit() const;

    // Орбита
    Vector3 m_target   {0.f, 0.f, 0.f};
    float   m_theta    = 0.8f;          // азимут
    float   m_phi      = 1.2f;          // зенит
    float   m_distance = 12.0f;

    // Проекция
    ProjectionType m_projType = ProjectionType::Perspective;
    float m_fovY   = 45.0f;
    float m_aspect = 1.0f;
    float m_near   = 0.1f;
    float m_far    = 500.0f;
    float m_orthoLeft = -10, m_orthoRight = 10, m_orthoBottom = -10, m_orthoTop = 10;

    // Кэш
    mutable Matrix4Raw m_view       {};
    mutable Matrix4Raw m_projection {};
    mutable Vector3    m_position   {};
    mutable bool m_viewDirty = true;
    mutable bool m_projDirty = true;
};

} // namespace MirEngine