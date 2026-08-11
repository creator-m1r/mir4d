// MirUI/Application/CADApplication.cpp
#include "CADApplication.hpp"
#include "../Core/Widget/WidgetFactory.hpp"
#include "../Widgets/Label/Label.hpp"
#include "../Widgets/Button/Button.hpp"

// Рендеринг и камера
#include "../../MirEngine/Camera/Camera.h"
#include "../../MirEngine/Camera/CameraController.h"
#include "../../MirEngine/Rendering/Core/RenderContext.h"
#include "../../MirEngine/Rendering/OpenGL/OpenGLRenderer.h"

#include <iostream>

namespace MirUI {

// ---------------------------------------------------------------------------
// Конструктор / деструктор
// ---------------------------------------------------------------------------
CADApplication::CADApplication() = default;

CADApplication::~CADApplication() {
    shutdown();
}

// ---------------------------------------------------------------------------
// Инициализация (принимает указатель на рендерер)
// ---------------------------------------------------------------------------
bool CADApplication::initialize(MirEngine::Rendering::OpenGLRenderer* renderer) {
    if (m_initialized) return true;

    // Сохраняем рендерер
    m_renderer = renderer;

    // ---------- Настройка камеры и контроллера ----------
    m_camera.setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 500.0f);
    m_camera.setOrbit(0.8f, 1.15f, 14.0f);
    m_camera.setTarget({0.0f, 0.0f, 0.0f});
    m_controller.setCamera(&m_camera);

    // Инициализация UI
    m_runtime = std::make_unique<UIRuntime>();
    if (!m_runtime->initialize()) {
        std::cerr << "[CADApplication] UIRuntime init failed\n";
        return false;
    }

    buildUI();
    m_initialized = true;
    std::cout << "[CADApplication] CAD UI ready\n";
    return true;
}

// ---------------------------------------------------------------------------
// Сеттер для рендерера (если не был передан в initialize)
// ---------------------------------------------------------------------------
void CADApplication::setRenderer(MirEngine::Rendering::OpenGLRenderer* renderer) {
    m_renderer = renderer;
}

// ---------------------------------------------------------------------------
// Построение UI (без изменений)
// ---------------------------------------------------------------------------
void CADApplication::buildUI() {
    auto rootOwned = std::make_unique<Container>(WidgetType::Window);
    rootOwned->setName("MainWindow");
    Widget* root = rootOwned.get();
    m_tree.setRoot(std::move(rootOwned));

    buildTopBar(root);
    buildSidebar(root);
    buildViewportArea(root);
    buildProperties(root);
    buildTimeline(root);
}

void CADApplication::buildTopBar(Widget* root) {
    auto* top = WidgetFactory::create(m_tree, WidgetType::Toolbar, "TopBar");
    top->setProperty("height", StateValue(64.0));

    auto* logo = WidgetFactory::create(m_tree, WidgetType::Label, "Logo");
    if (auto* lbl = dynamic_cast<Label*>(logo)) {
        lbl->setText("M1R.PRO – 4D САПР");
    }

    auto* b1 = WidgetFactory::create(m_tree, WidgetType::Button, "TabModel");
    if (auto* btn = dynamic_cast<Button*>(b1)) btn->setText("Модель");

    auto* b2 = WidgetFactory::create(m_tree, WidgetType::Button, "TabAssembly");
    if (auto* btn = dynamic_cast<Button*>(b2)) btn->setText("Сборка");

    auto* b3 = WidgetFactory::create(m_tree, WidgetType::Button, "Tab4D");
    if (auto* btn = dynamic_cast<Button*>(b3)) btn->setText("4D Тестирование");

    top->addChild(logo);
    top->addChild(b1);
    top->addChild(b2);
    top->addChild(b3);

    root->addChild(top);
}

void CADApplication::buildSidebar(Widget* root) {
    auto* sidebar = WidgetFactory::create(m_tree, WidgetType::DockPanel, "Sidebar");
    sidebar->setProperty("width", StateValue(280.0));

    m_projectTree = dynamic_cast<Tree*>(
        WidgetFactory::create(m_tree, WidgetType::Tree, "ProjectTree"));

    if (m_projectTree) {
        TreeNode project;
        project.id = "project";
        project.title = "Проект";
        project.expanded = true;

        TreeNode helmet;
        helmet.id = "helmet";
        helmet.title = "Космический шлем";
        helmet.expanded = true;

        TreeNode body{"body", "Корпус"};
        TreeNode visor{"visor", "Визор"};
        TreeNode life{"life", "Система жизнеобеспечения"};
        life.selected = true;

        helmet.children = { std::move(body), std::move(visor), std::move(life) };
        project.children.push_back(std::move(helmet));

        m_projectTree->addNode(std::move(project));
    }

    sidebar->addChild(m_projectTree);
    root->addChild(sidebar);
}

void CADApplication::buildViewportArea(Widget* root) {
    auto* area = WidgetFactory::create(m_tree, WidgetType::Panel, "ViewportArea");

    m_viewport = dynamic_cast<Viewport*>(
        WidgetFactory::create(m_tree, WidgetType::Viewport, "MainViewport"));

    if (m_viewport) {
        m_viewport->setViewportID("Perspective");
        m_viewport->setGridVisible(true);
        m_viewport->setAxesVisible(true);
        m_viewport->setGizmoVisible(true);
    }

    area->addChild(m_viewport);
    root->addChild(area);
}

void CADApplication::buildProperties(Widget* root) {
    auto* panel = WidgetFactory::create(m_tree, WidgetType::DockPanel, "PropertiesPanel");
    panel->setProperty("width", StateValue(320.0));

    m_properties = dynamic_cast<PropertyGrid*>(
        WidgetFactory::create(m_tree, WidgetType::PropertyGrid, "Inspector"));

    if (m_properties) {
        Property p;
        p.id = "type";     p.name = "Тип";      p.category = "Основные"; p.value = StateValue(std::string("Сборка"));
        m_properties->addProperty(p);
        p.id = "id";       p.name = "ID";       p.value = StateValue(std::string("ASM-EVA-0045"));
        m_properties->addProperty(p);
        p.id = "material"; p.name = "Материал"; p.value = StateValue(std::string("Ti-6Al-4V"));
        m_properties->addProperty(p);
        p.id = "status";   p.name = "Статус";   p.value = StateValue(std::string("В работе"));
        m_properties->addProperty(p);
    }

    panel->addChild(m_properties);
    root->addChild(panel);
}

void CADApplication::buildTimeline(Widget* root) {
    auto* timeline = WidgetFactory::create(m_tree, WidgetType::Panel, "Timeline");
    timeline->setProperty("height", StateValue(240.0));

    auto* label = WidgetFactory::create(m_tree, WidgetType::Label, "TimelineLabel");
    if (auto* lbl = dynamic_cast<Label*>(label)) {
        lbl->setText("4D Timeline / Gantt");
    }
    timeline->addChild(label);
    root->addChild(timeline);
}

// ---------------------------------------------------------------------------
// Игровой цикл
// ---------------------------------------------------------------------------
void CADApplication::update(double dt) {
    if (m_runtime) m_runtime->update(dt);
}

void CADApplication::render() {
    // 1. 3D-рендеринг сцены через OpenGLRenderer
    if (m_renderer) {
        MirEngine::Rendering::RenderContext ctx;
        ctx.updateMatrices(m_camera.getViewMatrix(), m_camera.getProjectionMatrix());
        auto pos = m_camera.getPosition();
        ctx.setCameraPosition(pos.x, pos.y, pos.z);

        m_renderer->render(ctx);
    }

    // 2. Рендеринг UI поверх 3D-сцены
    if (m_runtime) {
        m_runtime->render(m_tree);
    }
}

// ---------------------------------------------------------------------------
// Обработка ввода мыши (трансляция в CameraController)
// ---------------------------------------------------------------------------
void CADApplication::onMouseDown(int button, float x, float y) {
    m_controller.onMouseDown(button, x, y);
}

void CADApplication::onMouseMove(float x, float y) {
    m_controller.onMouseMove(x, y);
}

void CADApplication::onMouseUp(int button, float x, float y) {
    m_controller.onMouseUp(button, x, y);
}

void CADApplication::onMouseScroll(float delta) {
    m_controller.onMouseScroll(delta);
}

// ---------------------------------------------------------------------------
// Завершение работы
// ---------------------------------------------------------------------------
void CADApplication::shutdown() {
    if (!m_initialized) return;
    if (m_runtime) {
        m_runtime->shutdown();
        m_runtime.reset();
    }
    WidgetFactory::clearOwned();
    m_tree.setRoot(nullptr);
    m_projectTree = nullptr;
    m_properties  = nullptr;
    m_viewport    = nullptr;
    m_renderer    = nullptr;   // не владеем, просто обнуляем
    m_initialized = false;
}

// ---------------------------------------------------------------------------
// Загрузка модели / выбор объекта
// ---------------------------------------------------------------------------
void CADApplication::loadModel(const std::string& path) {
    std::cout << "[CADApplication] loadModel: " << path << "\n";
    // TODO: вызов MirEngine AssimpImporter + добавление в сцену
}

void CADApplication::selectObject(const std::string& id) {
    if (m_projectTree) {
        m_projectTree->select(id);
    }
}

} // namespace MirUI