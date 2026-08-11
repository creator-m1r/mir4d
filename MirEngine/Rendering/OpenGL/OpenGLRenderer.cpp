// MirEngine/Rendering/OpenGL/OpenGLRenderer.cpp
// =================================================================================
// Реализация OpenGLRenderer.
// =================================================================================

#include "OpenGLRenderer.h"
#include "OpenGLContext.h"
#include "OpenGLDevice.h"

#include "../Core/RenderDevice.h"   // ColorRGBA
#include "../Passes/GridPass.h"     // GridPass
#include "../Scene/Scene.h"         // Scene (заглушка)
#include "../Camera/Camera.h"       // Camera (заглушка)

#include <iostream>

namespace MirEngine {
namespace Rendering {

// --------------------------------------------------------------------------
// Конструктор / деструктор
// --------------------------------------------------------------------------
OpenGLRenderer::OpenGLRenderer(OpenGLContext* context)
    : m_context(context)
{
}

OpenGLRenderer::~OpenGLRenderer() = default;

// --------------------------------------------------------------------------
// Инициализация
// --------------------------------------------------------------------------
bool OpenGLRenderer::initialize()
{
    if (!m_context) {
        return false;
    }

    m_context->makeCurrent();

    m_device = std::make_unique<OpenGLDevice>(m_context);

    if (!m_device->initialize()) {
        m_device.reset();
        return false;
    }

    // Тёмный фон, удобный для CAD
    m_device->clear(ColorRGBA{0.055f, 0.065f, 0.085f, 1.0f});

    // -----------------------------------------------------------
    // Создание и инициализация GridPass
    // -----------------------------------------------------------
    m_gridPass = std::make_unique<GridPass>();
    if (!m_gridPass->initialize()) {
        std::cerr << "[OpenGLRenderer] GridPass init failed\n";
        // не фатально, рендеринг продолжится без сетки
    }

    m_initialized = true;
    return true;
}

// --------------------------------------------------------------------------
// Один кадр
// --------------------------------------------------------------------------
void OpenGLRenderer::render()
{
    if (!m_initialized || !m_device) {
        return;
    }

    m_device->beginFrame();

    // Очистка буфера (цвет + глубина)
    m_device->clear(ColorRGBA{0.055f, 0.065f, 0.085f, 1.0f});

    // Обновляем контекст (матрицы камеры — пока identity / тестовые)
    // В будущем брать из Camera / CADApplication
    RenderContext ctx;
    // TODO: ctx.updateMatrices(view, proj);

    if (m_gridPass && m_gridPass->isInitialized()) {
        // Scene и Camera пока заглушки — GridPass их не использует
        static Scene dummyScene;
        static Camera dummyCamera;
        m_gridPass->execute(ctx, dummyScene, dummyCamera, *m_device);
    }

    m_device->endFrame();   // внутри вызывает present()
}

// --------------------------------------------------------------------------
// Resize
// --------------------------------------------------------------------------
void OpenGLRenderer::resize(uint32_t width, uint32_t height)
{
    if (!m_initialized || !m_device) {
        return;
    }

    m_device->setViewportSize(width, height);
}

} // namespace Rendering
} // namespace MirEngine