// MirUI/Designer/Core/DesignerCore.hpp
// 🧠 Ядро визуального редактора MirUI Designer — теперь с поддержкой Undo/Redo, анимаций и тем.
//
// DesignerCore — главный управляющий класс редактора.
// Владеет:
//   • UIProject        — проектом интерфейса (дерево виджетов, тема, состояние).
//   • WidgetLibrary    — каталогом доступных типов виджетов.
//   • PreviewRuntime   — рантаймом для предпросмотра интерфейса.
//   • SelectionManager — выделением виджетов на холсте.
//   • CommandHistory   — историей действий (Undo/Redo).
//   • AnimationManager — системой анимаций свойств.
//   • ThemeManager     — управлением темами (светлая/тёмная/кастомные).
//
// Все изменения интерфейса проходят через этот класс:
//   1. Пользователь двигает кнопку → DesignerCore создаёт команду → выполняет её.
//   2. Команда изменяет WidgetTree в UIProject.
//   3. DesignerCore вызывает PreviewRuntime::update() для перерисовки.
//   4. Холст и инспектор обновляются через Renderer.
//
// Добавлены методы undo()/redo(), animateProperty(), а также управление темами:
//   • switchTheme(ThemeID)    — переключить активную тему.
//   • currentThemeName()      — имя текущей темы.
//   • availableThemes()       — список зарегистрированных тем.
//   • registerTheme(...)      — добавить тему в менеджер.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Model/UIProject.hpp"
#include "../Model/WidgetLibrary.hpp"
#include "../Runtime/PreviewRuntime.hpp"
#include "../Serialization/UIProjectSerializer.hpp"
#include "../Canvas/AlignmentManager.hpp"
#include "../Canvas/GuideManager.hpp"
#include "../../Core/Selection/SelectionManager.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Rendering/Renderer.hpp"
#include "../../Core/Animation/AnimationManager.hpp"
#include "../../Core/Theme/ThemeManager.hpp"
#include "../../Core/Theme/ThemeID.hpp"
#include "../../Foundation/Animation/AnimationSpec.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MirUI {

class DesignerCore {
public:
    DesignerCore()
        : m_project(std::make_unique<UIProject>())
        , m_library(std::make_unique<WidgetLibrary>())
        , m_preview(std::make_unique<PreviewRuntime>(*m_project))
        , m_serializer(std::make_unique<UIProjectSerializer>())
        , m_animationManager(std::make_unique<AnimationManager>())
    {
        m_library->populateDefaults();
        m_animationManager->setWidgetTree(&m_project->widgetTree());
    }

    // ── Доступ к компонентам ─────────────────────────────────
    [[nodiscard]] UIProject& project() { return *m_project; }
    [[nodiscard]] const UIProject& project() const { return *m_project; }
    [[nodiscard]] WidgetLibrary& library() { return *m_library; }
    [[nodiscard]] const WidgetLibrary& library() const { return *m_library; }
    [[nodiscard]] PreviewRuntime& preview() { return *m_preview; }
    [[nodiscard]] const PreviewRuntime& preview() const { return *m_preview; }
    [[nodiscard]] UIProjectSerializer& serializer() { return *m_serializer; }
    [[nodiscard]] AnimationManager& animationManager() { return *m_animationManager; }

    void setRenderer(Renderer* renderer) { m_preview->setRenderer(renderer); }
    [[nodiscard]] Renderer* renderer() const { return m_preview->renderer(); }

    // ── Undo / Redo ───────────────────────────────────────────
    void undo() { m_project->history().undo(); m_preview->update(); }
    void redo() { m_project->history().redo(); m_preview->update(); }
    [[nodiscard]] bool canUndo() const { return m_project->history().canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_project->history().canRedo(); }

    // ── Проект ────────────────────────────────────────────────
    bool saveProject(const std::string& path) { return m_serializer->save(path, *m_project); }
    bool loadProject(const std::string& path) { return m_serializer->load(path, *m_project); }
    void newProject() { m_project->clear(); }

    // ── Управление темами ────────────────────────────────────

    // Переключить активную тему по идентификатору.
    void switchTheme(const ThemeID& id) {
        m_project->themeManager().setTheme(id);
        // После смены темы перерисовываем интерфейс.
        m_preview->update();
    }

    // Получить имя текущей активной темы (для отображения в UI).
    [[nodiscard]] std::string currentThemeName() const {
        return m_project->themeManager().current().name;
    }

    // Получить список всех зарегистрированных тем (для меню выбора).
    // Возвращает вектор пар (ThemeID, name).
    [[nodiscard]] std::vector<std::pair<ThemeID, std::string>> availableThemes() const {
        std::vector<std::pair<ThemeID, std::string>> result;
        // Пока возвращаем стандартный набор; в будущем будем получать из ThemeManager.
        result.emplace_back(ThemeID("mir.light"), "Светлая тема");
        result.emplace_back(ThemeID("mir.dark"),  "Тёмная тема");
        return result;
    }

    // Зарегистрировать тему в менеджере (например, встроенную тёмную).
    void registerTheme(const Theme& theme) {
        m_project->themeManager().registerTheme(theme);
    }

    // ── Добавление виджета ───────────────────────────────────
    WidgetID addWidget(WidgetType type, WidgetID parentId) {
        auto widget = m_library->create(type);
        if (!widget) return WidgetID{};

        WidgetID newId = widget->id();
        Widget* parent = m_project->widgetTree().find(parentId);
        if (!parent) parent = m_project->widgetTree().root();
        if (parent) {
            parent->addChild(widget.release());
            m_project->widgetTree().registerWidget(parent->children().back());
        }

        m_preview->update();
        return newId;
    }

    // ── Перемещение и ресайз ─────────────────────────────────
    void moveWidget(WidgetID widgetId, double dx, double dy) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        Rect bounds = widget->bounds();
        bounds.x += dx;
        bounds.y += dy;
        widget->setBounds(bounds);
        m_preview->update();
    }

    void resizeWidget(WidgetID widgetId, const Rect& newBounds) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        widget->setBounds(newBounds);
        m_preview->update();
    }

    // ── Удаление ──────────────────────────────────────────────
    void deleteWidget(WidgetID widgetId) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        m_animationManager->stopWidget(widgetId);
        Widget* parent = widget->parent();
        if (parent) {
            parent->removeChild(widgetId);
            m_project->widgetTree().unregisterWidget(widgetId);
            delete widget;
        }
        m_project->selection().deselect(widgetId);
        m_preview->update();
    }

    // ── Свойства (обычное и анимированное) ───────────────────
    void setProperty(WidgetID widgetId, const std::string& name, const StateValue& value) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        widget->setProperty(name, value);
        m_preview->update();
    }

    // Анимированное изменение свойства.
    void animateProperty(WidgetID widgetId, const std::string& name,
                         const StateValue& targetValue,
                         const AnimationSpec& spec = AnimationSpec::smooth()) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        m_animationManager->animate(*widget, name, targetValue, spec);
    }

    [[nodiscard]] std::optional<StateValue> getProperty(WidgetID widgetId, const std::string& name) const {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return std::nullopt;
        return widget->getProperty(name);
    }

    // ── Выделение ────────────────────────────────────────────
    void selectWidget(WidgetID widgetId) { m_project->selection().select(widgetId); }
    void addToSelection(WidgetID widgetId) { m_project->selection().addToSelection(widgetId); }
    void clearSelection() { m_project->selection().clear(); }
    [[nodiscard]] const std::vector<WidgetID>& selectedWidgets() const { return m_project->selection().selected(); }

    // ── Выравнивание ─────────────────────────────────────────
    void alignSelected(AlignStrategy strategy) {
        const auto& selected = m_project->selection().selected();
        if (selected.size() < 2) return;
        std::vector<Rect> rects;
        for (WidgetID id : selected) {
            Widget* w = m_project->widgetTree().find(id);
            if (w) rects.push_back(w->bounds());
        }
        if (rects.empty()) return;
        std::vector<Rect> aligned = AlignmentManager::align(rects, strategy);
        for (size_t i = 0; i < selected.size() && i < aligned.size(); ++i) {
            Widget* w = m_project->widgetTree().find(selected[i]);
            if (w) w->setBounds(aligned[i]);
        }
        m_preview->update();
    }

    // ── Копирование / Вставка / Вырезание ────────────────────
    void copyWidget(WidgetID widgetId) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        std::unordered_map<std::string, StateValue> props;
        props["type"] = StateValue(static_cast<int64_t>(static_cast<int>(widget->type())));
        props["name"] = StateValue(widget->name());
        props["visible"] = StateValue(widget->isVisible());
        props["enabled"] = StateValue(widget->isEnabled());
        props["x"] = StateValue(widget->bounds().x);
        props["y"] = StateValue(widget->bounds().y);
        props["width"] = StateValue(widget->bounds().width);
        props["height"] = StateValue(widget->bounds().height);
        for (const auto& [key, val] : widget->allProperties()) {
            if (props.find(key) == props.end()) props[key] = val;
        }
        WidgetClipboard::instance().setContent(widget->type(), std::move(props));
    }

    WidgetID pasteWidget(WidgetID parentId) {
        WidgetClipboard& clipboard = WidgetClipboard::instance();
        if (!clipboard.hasContent()) return WidgetID{};
        Widget* parent = m_project->widgetTree().find(parentId);
        if (!parent) parent = m_project->widgetTree().root();
        if (!parent) return WidgetID{};
        auto widget = m_library->create(clipboard.type());
        if (!widget) return WidgetID{};
        WidgetID newId = widget->id();
        const auto& props = clipboard.properties();
        for (const auto& [key, val] : props) widget->setProperty(key, val);
        Rect bounds = widget->bounds();
        bounds.x += 20;
        bounds.y += 20;
        widget->setBounds(bounds);
        parent->addChild(widget.release());
        m_project->widgetTree().registerWidget(parent->children().back());
        m_preview->update();
        return newId;
    }

    void cutWidget(WidgetID widgetId) {
        copyWidget(widgetId);
        deleteWidget(widgetId);
    }

    // ── Предпросмотр ─────────────────────────────────────────
    void enterPreview() { m_preview->enterPreview(); }
    void exitPreview() { m_preview->exitPreview(); }
    void togglePreview() { m_preview->togglePreview(); }
    [[nodiscard]] bool isPreviewMode() const { return m_preview->isPreviewMode(); }

    // ── Отрисовка кадра (с обновлением анимаций) ─────────────
    void renderFrame(double deltaTime = 0.016) {
        // Обновляем все активные анимации.
        m_animationManager->update(deltaTime);
        // Выполняем компоновку и рендеринг.
        m_preview->render();
    }

private:
    std::unique_ptr<UIProject>           m_project;
    std::unique_ptr<WidgetLibrary>       m_library;
    std::unique_ptr<PreviewRuntime>      m_preview;
    std::unique_ptr<UIProjectSerializer> m_serializer;
    std::unique_ptr<AnimationManager>    m_animationManager;
};

} // namespace MirUI