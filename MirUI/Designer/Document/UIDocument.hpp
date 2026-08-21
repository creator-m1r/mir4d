
#pragma once

#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Core/Theme/ThemeManager.hpp"
#include "../../Core/State/StateStore.hpp"
#include "../../Core/Commands/CommandBus.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Selection/SelectionManager.hpp"
#include <memory>
#include <string>

namespace MirUI {

class UIDocument {
public:

    UIDocument()
        : m_widgetTree(std::make_unique<WidgetTree>())
        , m_themeManager(std::make_unique<ThemeManager>())
        , m_stateStore(std::make_unique<StateStore>())
        , m_commandBus(std::make_unique<CommandBus>())
        , m_history(std::make_unique<CommandHistory>())
        , m_selection(std::make_unique<SelectionManager>())
    {

        m_themeManager->resetToDefault();
    }

    [[nodiscard]] WidgetTree& widgetTree() { return *m_widgetTree; }
    [[nodiscard]] const WidgetTree& widgetTree() const { return *m_widgetTree; }

    [[nodiscard]] ThemeManager& themeManager() { return *m_themeManager; }
    [[nodiscard]] const ThemeManager& themeManager() const { return *m_themeManager; }

    [[nodiscard]] StateStore& stateStore() { return *m_stateStore; }
    [[nodiscard]] const StateStore& stateStore() const { return *m_stateStore; }

    [[nodiscard]] CommandBus& commandBus() { return *m_commandBus; }
    [[nodiscard]] const CommandBus& commandBus() const { return *m_commandBus; }

    [[nodiscard]] CommandHistory& history() { return *m_history; }
    [[nodiscard]] const CommandHistory& history() const { return *m_history; }

    [[nodiscard]] SelectionManager& selection() { return *m_selection; }
    [[nodiscard]] const SelectionManager& selection() const { return *m_selection; }

    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setFilePath(const std::string& path) { m_filePath = path; }
    [[nodiscard]] const std::string& filePath() const { return m_filePath; }

    void setModified(bool modified) { m_modified = modified; }
    [[nodiscard]] bool isModified() const { return m_modified; }

    void clear() {
        m_widgetTree = std::make_unique<WidgetTree>();
        m_themeManager->resetToDefault();
        m_stateStore->clear();
        m_commandBus = std::make_unique<CommandBus>();
        m_history->clear();
        m_selection->clear();
        m_name.clear();
        m_filePath.clear();
        m_modified = false;
    }

    bool save(const std::string& path) {

        m_filePath = path;
        m_modified = false;
        return true;
    }

    bool load(const std::string& path) {

        clear();
        m_filePath = path;
        m_modified = false;
        return true;
    }

private:
    std::unique_ptr<WidgetTree>       m_widgetTree;
    std::unique_ptr<ThemeManager>     m_themeManager;
    std::unique_ptr<StateStore>       m_stateStore;
    std::unique_ptr<CommandBus>       m_commandBus;
    std::unique_ptr<CommandHistory>   m_history;
    std::unique_ptr<SelectionManager> m_selection;

    std::string m_name;
    std::string m_filePath;
    bool m_modified = false;
};

}