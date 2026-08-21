// MirUI/Designer/Runtime/PreviewRuntime.hpp
// 🖥️ Рантайм предпросмотра — превращает UIProject в живые нативные Views.
//
// Когда пользователь нажимает кнопку "Preview" в MirUI Designer,
// PreviewRuntime берёт текущий проект (UIProject), извлекает из него
// WidgetTree и передаёт его в платформенный Renderer (SwiftUI, WinUI, WebUI).
//
// PreviewRuntime НЕ зависит от конкретного рендерера — он работает
// через абстрактный интерфейс Renderer. Благодаря этому один и тот же
// проект может быть показан:
//   • на macOS через SwiftUIRenderer,
//   • на Windows через WinUIRenderer,
//   • в браузере через WebUIRenderer (когда он появится).
//
// Основные обязанности:
//   1. Взять UIProject.
//   2. Выполнить компоновку (LayoutEngine), чтобы все размеры и позиции
//      были рассчитаны.
//   3. Передать подготовленное дерево виджетов в рендерер.
//   4. Управлять режимом предпросмотра (включить/выключить).
//
// PreviewRuntime не изменяет проект — он только читает его.
// Все изменения в проект вносятся через команды (AddWidgetCommand, …),
// после чего PreviewRuntime просто заново рендерит обновлённое дерево.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Model/UIProject.hpp"
#include "../../Core/Rendering/Renderer.hpp"
#include "../../Core/Layout/LayoutEngine.hpp"
#include <memory>

namespace MirUI {

class PreviewRuntime {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает проект, который будем показывать.
    explicit PreviewRuntime(UIProject& project)
        : m_project(project)
        , m_renderer(nullptr)
        , m_previewMode(false)
    {}

    // ── Подключение рендерера ────────────────────────────────
    // Здесь может быть SwiftUIRenderer, WinUIRenderer или NullRenderer.
    void setRenderer(Renderer* renderer) {
        m_renderer = renderer;
    }
    [[nodiscard]] Renderer* renderer() const { return m_renderer; }

    // ── Управление режимом предпросмотра ─────────────────────
    void enterPreview() {
        if (m_previewMode) return;
        m_previewMode = true;
        if (m_onEnterPreview) m_onEnterPreview();
        // При входе в предпросмотр сразу рендерим текущее состояние.
        render();
    }

    void exitPreview() {
        if (!m_previewMode) return;
        m_previewMode = false;
        if (m_onExitPreview) m_onExitPreview();
    }

    void togglePreview() {
        if (m_previewMode) {
            exitPreview();
        } else {
            enterPreview();
        }
    }

    [[nodiscard]] bool isPreviewMode() const { return m_previewMode; }

    // ── Колбэки для оповещения других компонентов ────────────
    // Например, DesignerCanvas может скрывать ручки выделения в режиме Preview.
    void setOnEnterPreview(std::function<void()> callback) { m_onEnterPreview = std::move(callback); }
    void setOnExitPreview(std::function<void()> callback)  { m_onExitPreview  = std::move(callback); }

    // ── Рендеринг ────────────────────────────────────────────
    // Выполняет компоновку и передаёт дерево виджетов рендереру.
    void render() {
        if (!m_renderer) return; // рендерер не подключён

        // Сначала выполняем компоновку, чтобы все размеры и позиции
        // были актуальными (могли измениться после команд редактирования).
        LayoutEngine engine;
        engine.layout(m_project.widgetTree());

        // Затем передаём подготовленное дерево рендереру.
        m_renderer->beginFrame();
        m_renderer->render(m_project.widgetTree());
        m_renderer->endFrame();
    }

    // ── Принудительное обновление (публичный метод) ──────────
    // Вызывается извне, когда проект изменился (например, после выполнения команды).
    void update() {
        if (m_previewMode) {
            render();
        }
    }

private:
    UIProject& m_project;
    Renderer*  m_renderer = nullptr;
    bool       m_previewMode = false;

    std::function<void()> m_onEnterPreview;
    std::function<void()> m_onExitPreview;
};

} // namespace MirUI