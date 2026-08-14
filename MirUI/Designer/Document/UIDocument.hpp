// MirUI/Designer/Document/UIDocument.hpp
// 📄 Универсальный документ интерфейса (UIDocument) — сердце редактора MirUI Designer.
// Хранит ВСЁ описание пользовательского интерфейса в одном объекте:
// - дерево виджетов (WidgetTree)
// - тему (Theme)
// - состояние (StateStore)
// - команды (CommandBus)
// - выделение (SelectionManager)
// - историю изменений (CommandHistory)
// 
// UIDocument НЕ зависит от платформы (ни SwiftUI, ни WinUI).
// Он используется как редактором (Designer), так и рендерерами.
// Именно этот объект сохраняется в файл .mirui и загружается обратно.
//
// Чистый C++23, без платформенных зависимостей.

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
    // ── Конструктор ──────────────────────────────────────────
    UIDocument()
        : m_widgetTree(std::make_unique<WidgetTree>())
        , m_themeManager(std::make_unique<ThemeManager>())
        , m_stateStore(std::make_unique<StateStore>())
        , m_commandBus(std::make_unique<CommandBus>())
        , m_history(std::make_unique<CommandHistory>())
        , m_selection(std::make_unique<SelectionManager>())
    {
        // Заряжаем тему по умолчанию.
        m_themeManager->resetToDefault();
    }

    // ── Основные компоненты (геттеры) ────────────────────────
    // Каждый компонент доступен по ссылке, чтобы можно было с ним работать напрямую.

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

    // ── Метаданные документа ─────────────────────────────────
    // Эти поля не влияют на отображение, но полезны для сохранения/загрузки.

    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setFilePath(const std::string& path) { m_filePath = path; }
    [[nodiscard]] const std::string& filePath() const { return m_filePath; }

    // Были ли несохранённые изменения с последнего сохранения?
    void setModified(bool modified) { m_modified = modified; }
    [[nodiscard]] bool isModified() const { return m_modified; }

    // ── Сброс документа (создание нового) ────────────────────
    // Удаляет всё, создаёт новое пустое дерево, сбрасывает тему и состояние.
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

    // ── Сохранение и загрузка (заглушки для будущего) ────────
    // В будущем здесь будет сериализация в файл .mirui.
    // Пока просто устанавливаем/сбрасываем флаг modified.

    bool save(const std::string& path) {
        // TODO: реализовать сохранение в файл (JSON / бинарный формат)
        m_filePath = path;
        m_modified = false;
        return true;
    }

    bool load(const std::string& path) {
        // TODO: реализовать загрузку из файла
        clear();
        m_filePath = path;
        m_modified = false;
        return true;
    }

private:
    std::unique_ptr<WidgetTree>       m_widgetTree;     // Дерево всех виджетов.
    std::unique_ptr<ThemeManager>     m_themeManager;   // Менеджер текущей темы.
    std::unique_ptr<StateStore>       m_stateStore;     // Хранилище состояний.
    std::unique_ptr<CommandBus>       m_commandBus;     // Шина команд.
    std::unique_ptr<CommandHistory>   m_history;        // История для Undo/Redo.
    std::unique_ptr<SelectionManager> m_selection;      // Менеджер выделения.

    std::string m_name;               // Имя документа (например, "MainWindow").
    std::string m_filePath;           // Путь к файлу, если документ был сохранён.
    bool m_modified = false;          // Есть ли несохранённые изменения.
};

} // namespace MirUI