// MirEngine/Scene/CameraController.cpp
#include "CameraController.h"

namespace MirEngine {

CameraController::CameraController(Camera* camera) : m_camera(camera) {}

void CameraController::onMouseDown(int button, float x, float y) {
    if (button == 0) m_leftDown   = true;
    if (button == 1) m_middleDown = true;
    if (button == 2) m_rightDown  = true;
    m_lastX = x;
    m_lastY = y;
}

void CameraController::onMouseUp(int button, float, float) {
    if (button == 0) m_leftDown   = false;
    if (button == 1) m_middleDown = false;
    if (button == 2) m_rightDown  = false;
}

void CameraController::onMouseMove(float x, float y) {
    if (!m_camera) return;

    const float dx = x - m_lastX;
    const float dy = y - m_lastY;
    m_lastX = x;
    m_lastY = y;

    // ЛКМ — орбита
    if (m_leftDown) {
        float theta = m_camera->getTheta() - dx * m_rotationSpeed;
        float phi   = m_camera->getPhi()   + dy * m_rotationSpeed;

        constexpr float kEps = 0.05f;
        constexpr float kPi  = 3.14159265f;
        phi = std::clamp(phi, kEps, kPi - kEps);

        m_camera->setOrbit(theta, phi, m_camera->getDistance());
    }

    // ПКМ или СКМ — pan
    if (m_rightDown || m_middleDown) {
        Vector3 right, up;
        getCameraVectors(right, up);

        // Масштабируем pan относительно дистанции (чтобы не «убегал» при большом зуме)
        const float scale = m_camera->getDistance() * m_panSpeed * 0.1f;

        Vector3 target = m_camera->getTarget();
        target.x -= right.x * dx * scale - up.x * dy * scale;
        target.y -= right.y * dx * scale - up.y * dy * scale;
        target.z -= right.z * dx * scale - up.z * dy * scale;
        m_camera->setTarget(target);
    }
}

void CameraController::onMouseScroll(float delta) {
    if (!m_camera) return;

    // delta > 0 → приблизить
    float dist = m_camera->getDistance() - delta * m_zoomSpeed * (m_camera->getDistance() * 0.1f);
    dist = std::max(0.3f, dist);
    m_camera->setDistance(dist);
}

void CameraController::update(double) {
    // Место для инерции / сглаживания
}

void CameraController::getCameraVectors(Vector3& outRight, Vector3& outUp) const {
    if (!m_camera) {
        outRight = {1,0,0};
        outUp = {0,1,0};
        return;
    }

    Vector3 pos = m_camera->getPosition();
    Vector3 target = m_camera->getTarget();
    Vector3 forward{target.x - pos.x, target.y - pos.y, target.z - pos.z};

    float len = std::sqrt(forward.x*forward.x + forward.y*forward.y + forward.z*forward.z);
    if (len > 1e-6f) {
        forward.x /= len; forward.y /= len; forward.z /= len;
    }

    Vector3 worldUp{0.f, 1.f, 0.f};

    // right = forward × worldUp  (или worldUp × forward — выбираем удобный)
    outRight = {
        forward.y * worldUp.z - forward.z * worldUp.y,
        forward.z * worldUp.x - forward.x * worldUp.z,
        forward.x * worldUp.y - forward.y * worldUp.x
    };
    len = std::sqrt(outRight.x*outRight.x + outRight.y*outRight.y + outRight.z*outRight.z);
    if (len > 1e-6f) {
        outRight.x /= len; outRight.y /= len; outRight.z /= len;
    }

    // up = right × forward
    outUp = {
        outRight.y * forward.z - outRight.z * forward.y,
        outRight.z * forward.x - outRight.x * forward.z,
        outRight.x * forward.y - outRight.y * forward.x
    };
}

} // namespace MirEngine