// MirEngine/Scene/CameraController.h
// =================================================================================
// Контроллер камеры для орбитального вращения, панорамирования и зума.
//
// Принимает события мыши (нажатие, перемещение, колёсико) и преобразует их
// в изменение параметров орбиты камеры (theta, phi, distance, target).
// Не зависит от платформы — конкретные UI-события транслируются в вызовы методов.
//
// Архитектура:
//   Viewport получает события от MirUI (SwiftUI/WinUI), вызывает методы контроллера.
//   Контроллер обновляет Camera, которая используется при рендеринге кадра.
//
// Управление:
//   - ЛКМ + перемещение = вращение вокруг target (изменение theta и phi).
//   - ПКМ + перемещение = панорамирование (сдвиг target в плоскости экрана).
//   - Колёсико мыши = зум (изменение distance).
// =================================================================================

// MirEngine/Scene/CameraController.h
#pragma once

#include "Camera.h"
#include <algorithm>
#include <cmath>

namespace MirEngine {

class CameraController {
public:
    explicit CameraController(Camera* camera = nullptr);

    void setCamera(Camera* camera) noexcept { m_camera = camera; }
    [[nodiscard]] Camera* camera() const noexcept { return m_camera; }

    // События мыши (координаты в пикселях вьюпорта)
    // button: 0 = ЛКМ, 1 = СКМ, 2 = ПКМ
    void onMouseDown(int button, float x, float y);
    void onMouseUp(int button, float x, float y);
    void onMouseMove(float x, float y);
    void onMouseScroll(float delta);          // >0 = приблизить

    void update(double deltaTime = 0.0);      // для будущей инерции

    // Чувствительность
    void setRotationSpeed(float s) noexcept { m_rotationSpeed = s; }
    void setPanSpeed(float s)      noexcept { m_panSpeed = s; }
    void setZoomSpeed(float s)     noexcept { m_zoomSpeed = s; }

private:
    void getCameraVectors(Vector3& outRight, Vector3& outUp) const;

    Camera* m_camera = nullptr;

    bool  m_leftDown  = false;
    bool  m_middleDown = false;
    bool  m_rightDown = false;

    float m_lastX = 0.f;
    float m_lastY = 0.f;

    float m_rotationSpeed = 0.005f;
    float m_panSpeed      = 0.015f;
    float m_zoomSpeed     = 0.8f;
};

} // namespace MirEngine