// MirUI/Application/CADApplication.hpp
#pragma once

#include "../Core/Widget/WidgetTree.hpp"
#include "../Core/Runtime/UIRuntime.hpp"
#include "../Widgets/Tree/Tree.hpp"
#include "../Widgets/PropertyGrid/PropertyGrid.hpp"
#include "../Widgets/Viewport/Viewport.hpp"
#include "../Widgets/Toolbar/Toolbar.hpp"
#include "../Widgets/Container/Container.hpp"

// Новые зависимости для 3D-рендеринга
#include "../../MirEngine/Camera/Camera.h"
#include "../../MirEngine/Camera/CameraController.h"
#include "../../MirEngine/Rendering/Core/RenderContext.h"

#include <memory>
#include <string>

namespace MirEngine {
namespace Rendering {
    class OpenGLRenderer;   // forward declaration
}
}

namespace MirUI {

class CADApplication {
public:
    CADApplication();
    ~CADApplication();

    // Инициализация с указателем на рендерер (передаётся из платформенного слоя)
    bool initialize(MirEngine::Rendering::OpenGLRenderer* renderer = nullptr);
    void update(double deltaTime);
    void render();                // собирает матрицы камеры и вызывает рендерер
    void shutdown();

    // Сеттер для рендерера (если не был передан в initialize)
    void setRenderer(MirEngine::Rendering::OpenGLRenderer* renderer);

    // Обработка ввода мыши (транслируется в CameraController)
    void onMouseDown(int button, float x, float y);
    void onMouseMove(float x, float y);
    void onMouseUp(int button, float x, float y);
    void onMouseScroll(float delta);

    [[nodiscard]] WidgetTree& tree() noexcept { return m_tree; }
    [[nodiscard]] const WidgetTree& tree() const noexcept { return m_tree; }

    void loadModel(const std::string& path);
    void selectObject(const std::string& id);

private:
    void buildUI();
    void buildTopBar(Widget* root);
    void buildSidebar(Widget* root);
    void buildViewportArea(Widget* root);
    void buildProperties(Widget* root);
    void buildTimeline(Widget* root);

    WidgetTree m_tree;
    std::unique_ptr<UIRuntime> m_runtime;

    Tree*         m_projectTree = nullptr;
    PropertyGrid* m_properties  = nullptr;
    Viewport*     m_viewport    = nullptr;

    // 3D-камера и контроллер орбиты
    MirEngine::Camera           m_camera;
    MirEngine::CameraController m_controller{&m_camera};

    // Рендерер (не владеет)
    MirEngine::Rendering::OpenGLRenderer* m_renderer = nullptr;

    bool m_initialized = false;
};

} // namespace MirUI