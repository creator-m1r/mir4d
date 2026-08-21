
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

    void undo() { m_project->history().undo(); m_preview->update(); }
    void redo() { m_project->history().redo(); m_preview->update(); }
    [[nodiscard]] bool canUndo() const { return m_project->history().canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_project->history().canRedo(); }

    bool saveProject(const std::string& path) { return m_serializer->save(path, *m_project); }
    bool loadProject(const std::string& path) { return m_serializer->load(path, *m_project); }
    void newProject() { m_project->clear(); }

    void switchTheme(const ThemeID& id) {
        m_project->themeManager().setTheme(id);

        m_preview->update();
    }

    [[nodiscard]] std::string currentThemeName() const {
        return m_project->themeManager().current().name;
    }

    [[nodiscard]] std::vector<std::pair<ThemeID, std::string>> availableThemes() const {
        std::vector<std::pair<ThemeID, std::string>> result;

        result.emplace_back(ThemeID("mir.light"), "Светлая тема");
        result.emplace_back(ThemeID("mir.dark"),  "Тёмная тема");
        return result;
    }

    void registerTheme(const Theme& theme) {
        m_project->themeManager().registerTheme(theme);
    }

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

    void setProperty(WidgetID widgetId, const std::string& name, const StateValue& value) {
        Widget* widget = m_project->widgetTree().find(widgetId);
        if (!widget) return;
        widget->setProperty(name, value);
        m_preview->update();
    }

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

    void selectWidget(WidgetID widgetId) { m_project->selection().select(widgetId); }
    void addToSelection(WidgetID widgetId) { m_project->selection().addToSelection(widgetId); }
    void clearSelection() { m_project->selection().clear(); }
    [[nodiscard]] const std::vector<WidgetID>& selectedWidgets() const { return m_project->selection().selected(); }

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

    void enterPreview() { m_preview->enterPreview(); }
    void exitPreview() { m_preview->exitPreview(); }
    void togglePreview() { m_preview->togglePreview(); }
    [[nodiscard]] bool isPreviewMode() const { return m_preview->isPreviewMode(); }

    void renderFrame(double deltaTime = 0.016) {

        m_animationManager->update(deltaTime);

        m_preview->render();
    }

private:
    std::unique_ptr<UIProject>           m_project;
    std::unique_ptr<WidgetLibrary>       m_library;
    std::unique_ptr<PreviewRuntime>      m_preview;
    std::unique_ptr<UIProjectSerializer> m_serializer;
    std::unique_ptr<AnimationManager>    m_animationManager;
};

}