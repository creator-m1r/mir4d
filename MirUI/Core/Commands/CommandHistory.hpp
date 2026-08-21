
#pragma once

#include "Command.hpp"
#include <vector>
#include <memory>
#include <cstddef>

namespace MirUI {

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual bool execute() = 0;

    virtual bool undo() = 0;

    [[nodiscard]] virtual std::string description() const {
        return "Команда без описания";
    }
};

class CommandHistory {
public:

    void execute(std::unique_ptr<ICommand> command) {
        if (!command) return;

        if (!command->execute()) {
            return;
        }

        if (m_currentIndex < m_history.size() - 1) {
            m_history.erase(m_history.begin() + m_currentIndex + 1, m_history.end());
        }

        m_history.push_back(std::move(command));
        m_currentIndex = m_history.size() - 1;
    }

    bool undo() {
        if (m_currentIndex == static_cast<size_t>(-1) || m_history.empty()) {
            return false;
        }

        if (!m_history[m_currentIndex]->undo()) {
            return false;
        }

        --m_currentIndex;
        return true;
    }

    bool redo() {
        if (m_currentIndex + 1 >= m_history.size()) {
            return false;
        }

        ++m_currentIndex;
        if (!m_history[m_currentIndex]->execute()) {

            --m_currentIndex;
            return false;
        }
        return true;
    }

    [[nodiscard]] bool canUndo() const {
        return m_currentIndex != static_cast<size_t>(-1) && !m_history.empty();
    }

    [[nodiscard]] bool canRedo() const {
        return m_currentIndex + 1 < m_history.size();
    }

    void clear() {
        m_history.clear();
        m_currentIndex = static_cast<size_t>(-1);
    }

    [[nodiscard]] std::string undoDescription() const {
        if (!canUndo()) return "";
        return m_history[m_currentIndex]->description();
    }

    [[nodiscard]] std::string redoDescription() const {
        if (!canRedo()) return "";
        return m_history[m_currentIndex + 1]->description();
    }

private:

    std::vector<std::unique_ptr<ICommand>> m_history;

    size_t m_currentIndex = static_cast<size_t>(-1);
};

}