
#pragma once

#include "Widget/WidgetTree.hpp"
#include "Theme/ThemeManager.hpp"
#include "State/StateStore.hpp"
#include "Commands/CommandHistory.hpp"
#include "Selection/SelectionManager.hpp"
#include "Layout/LayoutEngine.hpp"
#include "Animation/AnimationManager.hpp"
#include <memory>

namespace MirUI {

class UIContext {
public:
    UIContext()
        : m_widgetTree(std::make_unique<WidgetTree>())
        , m_themeManager(std::make_unique<ThemeManager>())
        , m_stateStore(std::make_unique<StateStore>())
        , m_history(std::make_unique<CommandHistory>())
        , m_selection(std::make_unique<SelectionManager>())
        , m_animationManager(std::make_unique<AnimationManager>())
    {

        m_animationManager->setWidgetTree(m_widgetTree.get());
        
        auto window = WidgetFactory::create(WidgetType::Window);
        if (window) {
            window->setName("MainWindow");
            window->setLayoutData(LayoutData::fixed(1024, 768));
            m_widgetTree->setRoot(std::move(window));
        }
    }

    [[nodiscard]] WidgetTree& widgetTree() { return *m_widgetTree; }
    [[nodiscard]] const WidgetTree& widgetTree() const { return *m_widgetTree; }

    [[nodiscard]] ThemeManager& themeManager() { return *m_themeManager; }
    [[nodiscard]] const ThemeManager& themeManager() const { return *m_themeManager; }

    [[nodiscard]] StateStore& stateStore() { return *m_stateStore; }
    [[nodiscard]] const StateStore& stateStore() const { return *m_stateStore; }

    [[nodiscard]] CommandHistory& history() { return *m_history; }
    [[nodiscard]] const CommandHistory& history() const { return *m_history; }

    [[nodiscard]] SelectionManager& selection() { return *m_selection; }
    [[nodiscard]] const SelectionManager& selection() const { return *m_selection; }

    [[nodiscard]] AnimationManager& animationManager() { return *m_animationManager; }
    [[nodiscard]] const AnimationManager& animationManager() const { return *m_animationManager; }

    WidgetID addWidget(WidgetType type, WidgetID parentId) {
        auto widget = WidgetFactory::create(type);
        if (!widget) return WidgetID{};
        
        WidgetID newId = widget->id();
        Widget* parent = m_widgetTree->find(parentId);
        if (!parent) parent = m_widgetTree->root();
        if (parent) {
            parent->addChild(widget.release());
            m_widgetTree->registerWidget(parent->children().back());
        }
        return newId;
    }

    void switchTheme(const ThemeID& id) {
        m_themeManager->setTheme(id);
    }

    void undo() { m_history->undo(); }
    void redo() { m_history->redo(); }

    void layout() {
        LayoutEngine engine;
        engine.layout(*m_widgetTree);
    }

    void update(double deltaTime = 0.016) {
        m_animationManager->update(deltaTime);
        layout();
    }

private:
    std::unique_ptr<WidgetTree>        m_widgetTree;
    std::unique_ptr<ThemeManager>      m_themeManager;
    std::unique_ptr<StateStore>        m_stateStore;
    std::unique_ptr<CommandHistory>    m_history;
    std::unique_ptr<SelectionManager>  m_selection;
    std::unique_ptr<AnimationManager>  m_animationManager;

};

}