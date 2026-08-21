// MirUI/Core/Commands/CommandHistory.hpp
// 💾 История выполненных команд — сердце системы Undo / Redo.
// Каждое действие в редакторе (добавил кнопку, изменил текст, передвинул панель)
// оформляется как отдельная команда. История хранит их стек и позволяет
// откатывать (Undo) и повторять (Redo) действия.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Command.hpp"        // наш базовый интерфейс ICommand (см. ниже)
#include <vector>
#include <memory>
#include <cstddef>            // для size_t

namespace MirUI {

// ── Базовый интерфейс команды ─────────────────────────────
// Каждая конкретная команда (AddWidget, DeleteWidget, MoveWidget и т.д.)
// будет наследоваться от этого класса и реализовывать execute() и undo().
// Пока что он находится здесь же, в заголовке истории, чтобы всё было рядом.
// В будущем его можно вынести в отдельный файл Command.hpp.

class ICommand {
public:
    virtual ~ICommand() = default;

    // Выполнить команду (вперёд). Возвращает true, если выполнение прошло успешно.
    virtual bool execute() = 0;

    // Откатить команду (назад). Возвращает true, если откат выполнен.
    virtual bool undo() = 0;

    // Краткое описание команды для отображения в UI (например, "Добавить кнопку").
    [[nodiscard]] virtual std::string description() const {
        return "Команда без описания";
    }
};

// ── Сама история команд ───────────────────────────────────

class CommandHistory {
public:
    // Выполнить новую команду и поместить её в историю.
    // После этого все «отменённые» команды (которые были впереди по redo) теряются.
    void execute(std::unique_ptr<ICommand> command) {
        if (!command) return;

        // Выполняем команду. Если она не смогла выполниться — не добавляем в историю.
        if (!command->execute()) {
            return; // команда не выполнена, ничего не делаем
        }

        // Удаляем все команды, которые были "впереди" (после текущей позиции),
        // потому что новое действие создаёт новую ветку истории.
        if (m_currentIndex < m_history.size() - 1) {
            m_history.erase(m_history.begin() + m_currentIndex + 1, m_history.end());
        }

        // Добавляем команду в конец истории.
        m_history.push_back(std::move(command));
        m_currentIndex = m_history.size() - 1; // теперь мы в конце
    }

    // Отменить последнюю команду (Undo). Возвращает true, если откат выполнен.
    bool undo() {
        if (m_currentIndex == static_cast<size_t>(-1) || m_history.empty()) {
            return false; // нечего отменять
        }

        // Откатываем текущую команду.
        if (!m_history[m_currentIndex]->undo()) {
            return false; // откат не удался
        }

        // Сдвигаем указатель на одну позицию назад.
        --m_currentIndex;
        return true;
    }

    // Повторить ранее отменённую команду (Redo). Возвращает true, если повтор выполнен.
    bool redo() {
        if (m_currentIndex + 1 >= m_history.size()) {
            return false; // нечего повторять (мы в конце истории)
        }

        // Перемещаемся вперёд и выполняем команду заново.
        ++m_currentIndex;
        if (!m_history[m_currentIndex]->execute()) {
            // Если выполнить не удалось, откатываемся обратно.
            --m_currentIndex;
            return false;
        }
        return true;
    }

    // Можно ли сейчас выполнить Undo?
    [[nodiscard]] bool canUndo() const {
        return m_currentIndex != static_cast<size_t>(-1) && !m_history.empty();
    }

    // Можно ли сейчас выполнить Redo?
    [[nodiscard]] bool canRedo() const {
        return m_currentIndex + 1 < m_history.size();
    }

    // Очистить всю историю (например, при создании нового документа).
    void clear() {
        m_history.clear();
        m_currentIndex = static_cast<size_t>(-1); // нет текущей команды
    }

    // Получить описание команды, которая будет отменена при следующем Undo.
    [[nodiscard]] std::string undoDescription() const {
        if (!canUndo()) return "";
        return m_history[m_currentIndex]->description();
    }

    // Получить описание команды, которая будет повторена при следующем Redo.
    [[nodiscard]] std::string redoDescription() const {
        if (!canRedo()) return "";
        return m_history[m_currentIndex + 1]->description();
    }

private:
    // Все выполненные команды в хронологическом порядке.
    std::vector<std::unique_ptr<ICommand>> m_history;

    // Индекс текущей (последней выполненной) команды.
    // -1 означает, что история пуста.
    size_t m_currentIndex = static_cast<size_t>(-1);
};

} // namespace MirUI