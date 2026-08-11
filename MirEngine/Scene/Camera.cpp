// MirEngine/Scene/Camera.cpp
// =================================================================================
// Реализация камеры.
// =================================================================================

// MirEngine/Scene/Camera.cpp
#include "Camera.h"
#include <cstring>

namespace MirEngine {

static Matrix4Raw identity() {
    return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
}

Camera::Camera() {
    m_view = identity();
    m_projection = identity();
    updatePositionFromOrbit();
}

void Camera::setPerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane) {
    m_projType = ProjectionType::Perspective;
    m_fovY = fovYDegrees;
    m_aspect = aspect > 0.f ? aspect : 1.f;
    m_near = nearPlane;
    m_far = farPlane;
    m_projDirty = true;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_projType = ProjectionType::Orthographic;
    m_orthoLeft = left; m_orthoRight = right;
    m_orthoBottom = bottom; m_orthoTop = top;
    m_near = nearPlane; m_far = farPlane;
    m_projDirty = true;
}

void Camera::setAspect(float aspect) {
    m_aspect = aspect > 0.f ? aspect : 1.f;
    m_projDirty = true;
}

void Camera::setTarget(const Vector3& target) {
    m_target = target;
    m_viewDirty = true;
}

void Camera::setOrbit(float theta, float phi, float distance) {
    m_theta = theta;
    m_phi = phi;
    m_distance = std::max(0.1f, distance);
    m_viewDirty = true;
}

void Camera::setDistance(float distance) {
    m_distance = std::max(0.1f, distance);
    m_viewDirty = true;
}

void Camera::setTheta(float theta) {
    m_theta = theta;
    m_viewDirty = true;
}

void Camera::setPhi(float phi) {
    m_phi = phi;
    m_viewDirty = true;
}

void Camera::updatePositionFromOrbit() const {
    // Сферические → декартовы
    const float sinPhi = std::sin(m_phi);
    const float cosPhi = std::cos(m_phi);
    const float sinTheta = std::sin(m_theta);
    const float cosTheta = std::cos(m_theta);

    m_position.x = m_target.x + m_distance * sinPhi * sinTheta;
    m_position.y = m_target.y + m_distance * cosPhi;
    m_position.z = m_target.z + m_distance * sinPhi * cosTheta;
}

Vector3 Camera::getPosition() const {
    if (m_viewDirty) updatePositionFromOrbit();
    return m_position;
}

void Camera::updateViewMatrix() const {
    updatePositionFromOrbit();

    // lookAt (right-handed, column-major)
    Vector3 eye = m_position;
    Vector3 center = m_target;
    Vector3 up{0.f, 1.f, 0.f};

    Vector3 f{center.x - eye.x, center.y - eye.y, center.z - eye.z};
    float len = std::sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
    if (len > 1e-6f) { f.x/=len; f.y/=len; f.z/=len; }

    // s = f × up
    Vector3 s{f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x};
    len = std::sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
    if (len > 1e-6f) { s.x/=len; s.y/=len; s.z/=len; }

    // u = s × f
    Vector3 u{s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x};

    m_view = {
        s.x, u.x, -f.x, 0.f,
        s.y, u.y, -f.y, 0.f,
        s.z, u.z, -f.z, 0.f,
        -(s.x*eye.x + s.y*eye.y + s.z*eye.z),
        -(u.x*eye.x + u.y*eye.y + u.z*eye.z),
        (f.x*eye.x + f.y*eye.y + f.z*eye.z),
        1.f
    };
    m_viewDirty = false;
}

void Camera::updateProjectionMatrix() const {
    if (m_projType == ProjectionType::Perspective) {
        const float f = 1.f / std::tan(m_fovY * 0.5f * 3.14159265f / 180.f);
        const float nf = 1.f / (m_near - m_far);

        m_projection = {
            f / m_aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (m_far + m_near) * nf, -1.f,
            0, 0, 2.f * m_far * m_near * nf, 0
        };
    } else {
        const float rl = m_orthoRight - m_orthoLeft;
        const float tb = m_orthoTop - m_orthoBottom;
        const float fn = m_far - m_near;

        m_projection = {
            2.f/rl, 0, 0, 0,
            0, 2.f/tb, 0, 0,
            0, 0, -2.f/fn, 0,
            -(m_orthoRight+m_orthoLeft)/rl,
            -(m_orthoTop+m_orthoBottom)/tb,
            -(m_far+m_near)/fn,
            1.f
        };
    }
    m_projDirty = false;
}

Matrix4Raw Camera::getViewMatrix() const {
    if (m_viewDirty) updateViewMatrix();
    return m_view;
}

Matrix4Raw Camera::getProjectionMatrix() const {
    if (m_projDirty) updateProjectionMatrix();
    return m_projection;
}

void Camera::getRayFromScreen(float screenX, float screenY,
                              uint32_t vpWidth, uint32_t vpHeight,
                              Vector3& outOrigin, Vector3& outDirection) const {
    // NDC
    float x = (2.f * screenX) / vpWidth - 1.f;
    float y = 1.f - (2.f * screenY) / vpHeight;

    outOrigin = getPosition();

    // Упрощённо: направление через unproject (можно расширить)
    Matrix4Raw invView = getViewMatrix(); // для простоты пока не инвертируем полностью
    (void)invView;
    Vector3 target = m_target;
    outDirection = {target.x - outOrigin.x, target.y - outOrigin.y, target.z - outOrigin.z};
    float len = std::sqrt(outDirection.x*outDirection.x + outDirection.y*outDirection.y + outDirection.z*outDirection.z);
    if (len > 1e-6f) {
        outDirection.x /= len; outDirection.y /= len; outDirection.z /= len;
    }
    (void)x; (void)y;
}

} // namespace MirEngine