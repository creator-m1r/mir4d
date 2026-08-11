// MirEngine/Viewport/Viewport.cpp
// =================================================================================
// Реализация Viewport.
// =================================================================================

#include "Viewport.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Core/RenderContext.h"
#include "../Scene/Camera.h"
#include "../Scene/Scene.h"
#include "CameraController.h"   // <-- добавлено
#include "../Selection/SelectionManager.h" // <-- добавлено (требуется для onMouseClick)
#include <spdlog/spdlog.h>

namespace MirEngine {

Viewport::Viewport(Rendering::Renderer* renderer, uint32_t width, uint32_t height)
    : m_renderer(renderer)
    , m_selectionManager(nullptr) // инициализируем при необходимости
{
    m_context = std::make_unique<Rendering::RenderContext>();
    resize(width, height);
    spdlog::info("[Viewport] Created with size {}x{}.", width, height);
}

void Viewport::setCamera(Camera* camera) {
    m_camera = camera;
}

void Viewport::setScene(Scene* scene) {
    m_scene = scene;
}

void Viewport::resize(uint32_t width, uint32_t height) {
    m_context->viewportWidth = width;
    m_context->viewportHeight = height;
    m_context->aspectRatio = (height > 0) ? static_cast<float>(width) / height : 1.0f;
    if (m_renderer) {
        m_renderer->resize(width, height);
    }
}

void Viewport::render() {
    if (!m_renderer) {
        spdlog::error("[Viewport] Renderer is null.");
        return;
    }
    if (!m_camera) {
        spdlog::warn("[Viewport] No camera set, skipping render.");
        return;
    }
    if (!m_scene) {
        spdlog::warn("[Viewport] No scene set, skipping render.");
        return;
    }

    // Обновляем контекст из камеры (предполагается, что у Camera есть метод getViewMatrix, getProjectionMatrix)
    m_context->updateMatrices(m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
    m_context->setCameraPosition(m_camera->getPosition().x, m_camera->getPosition().y, m_camera->getPosition().z);

    // Выполняем рендеринг
    m_renderer->render(*m_scene, *m_camera, *m_context);
}

// ---------------------------------------------------------------------------------
// Новый метод: обработка клика мыши для выделения объектов
// ---------------------------------------------------------------------------------
void Viewport::onMouseClick(float x, float y, bool addToSelection) {
    if (!m_camera || !m_scene || !m_selectionManager) return;

    Vector3 origin, direction;
    m_camera->getRayFromScreen(x, y, 
                               m_context->viewportWidth, 
                               m_context->viewportHeight,
                               origin, direction);
    m_selectionManager->pickAndSelect(*m_scene, origin, direction, addToSelection);
}

} // namespace MirEngine