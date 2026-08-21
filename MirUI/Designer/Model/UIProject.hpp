// MirUI/Designer/Model/UIProject.hpp
// 📁 Обобщённая модель проекта — чистые данные, без логики редактора.
//
// UIProject хранит:
//   • WidgetTree      — дерево виджетов
//   • ThemeManager    — темы (активная + зарегистрированные)
//   • StateStore      — состояние интерфейса
//   • CommandHistory  — история команд (Undo/Redo)
//   • SelectionManager — текущее выделение
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <memory>

#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"      // <-- нужно для WidgetFactory::create
#include "../../Core/Theme/ThemeManager.hpp"
#include "../../Core/State/StateStore.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Selection/SelectionManager.hpp"

namespace MirUI {

class UIProject {
public:
    // ── Конструктор ──────────────────────────────────────────
    UIProject()
        : m_id("untitled")
        , m_name("Новый проект")
        , m_widgetTree(std::make_unique<WidgetTree>())
        , m_themeManager(std::make_unique<ThemeManager>())
        , m_stateStore(std::make_unique<StateStore>())
        , m_history(std::make_unique<CommandHistory>())
        , m_selection(std::make_unique<SelectionManager>())
    {
        // Создаём корневое окно
        auto window = WidgetFactory::create(WidgetType::Window);
        if (window) {
            window->setName("MainWindow");
            window->setLayoutData(LayoutData::fixed(1024, 768));
            m_widgetTree->setRoot(std::move(window));
        }
    }

    // ── Идентификаторы ──────────────────────────────────────
    [[nodiscard]] const std::string& id() const { return m_id; }
    void setId(const std::string& id) { m_id = id; }

    [[nodiscard]] const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    // ── Компоненты ───────────────────────────────────────────
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

    // ── Информация о файле ──────────────────────────────────
    void setFilePath(const std::string& path) { m_filePath = path; }
    [[nodiscard]] const std::string& filePath() const { return m_filePath; }

    // ── Сброс ────────────────────────────────────────────────
    void clear() {
        m_widgetTree = std::make_unique<WidgetTree>();
        m_themeManager->resetToDefault();
        m_stateStore->clear();
        m_history->clear();
        m_selection->clear();
        m_filePath.clear();

        auto window = WidgetFactory::create(WidgetType::Window);
        if (window) {
            window->setName("MainWindow");
            window->setLayoutData(LayoutData::fixed(1024, 768));
            m_widgetTree->setRoot(std::move(window));
        }
    }

private:
    std::string m_id;
    std::string m_name;
    std::string m_filePath;

    std::unique_ptr<WidgetTree>       m_widgetTree;
    std::unique_ptr<ThemeManager>     m_themeManager;
    std::unique_ptr<StateStore>       m_stateStore;
    std::unique_ptr<CommandHistory>   m_history;
    std::unique_ptr<SelectionManager> m_selection;
};

} // namespace MirUI