
#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../Document/UIDocument.hpp"
#include <memory>

namespace MirUI {

class DesignerHistory {
public:

    explicit DesignerHistory(UIDocument& document)
        : m_document(document)
        , m_history(std::make_unique<CommandHistory>())
        , m_savedIndex(static_cast<size_t>(-1))
    {

        m_document.setModified(false);
    }

    void execute(std::unique_ptr<ICommand> command) {
        if (!command) return;

        m_history->execute(std::move(command));

        updateModifiedFlag();
    }

    bool undo() {
        if (!m_history->canUndo()) return false;

        bool result = m_history->undo();
        if (result) {
            updateModifiedFlag();
        }
        return result;
    }

    bool redo() {
        if (!m_history->canRedo()) return false;

        bool result = m_history->redo();
        if (result) {
            updateModifiedFlag();
        }
        return result;
    }

    void markSaved() {

        if (m_history->canUndo() || m_history->canRedo()) {

            m_savedIndex = m_currentIndex;
        } else {
            m_savedIndex = static_cast<size_t>(-1);
        }
        m_document.setModified(false);
    }

    [[nodiscard]] bool isModified() const {

        if (m_savedIndex == static_cast<size_t>(-1)) {
            return m_history->canUndo();
        }

        return m_currentIndex != m_savedIndex;
    }

    [[nodiscard]] CommandHistory& history() { return *m_history; }
    [[nodiscard]] const CommandHistory& history() const { return *m_history; }

    void clear() {
        m_history->clear();
        m_savedIndex = static_cast<size_t>(-1);
        m_currentIndex = static_cast<size_t>(-1);
        m_document.setModified(false);
    }

    [[nodiscard]] std::string undoDescription() const { return m_history->undoDescription(); }
    [[nodiscard]] std::string redoDescription() const { return m_history->redoDescription(); }
    [[nodiscard]] bool canUndo() const { return m_history->canUndo(); }
    [[nodiscard]] bool canRedo() const { return m_history->canRedo(); }

private:
    UIDocument& m_document;
    std::unique_ptr<CommandHistory> m_history;

    size_t m_savedIndex = static_cast<size_t>(-1);

    size_t m_currentIndex = static_cast<size_t>(-1);

    void updateModifiedFlag() {

        m_document.setModified(true);
    }
};

}