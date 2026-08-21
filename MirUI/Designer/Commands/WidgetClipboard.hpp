// MirUI/Designer/Commands/WidgetClipboard.hpp
// 📋 Внутренний буфер обмена для виджетов MirUI Designer.
//
// Когда пользователь копирует кнопку (Cmd+C), её свойства сохраняются
// в этот синглтон-объект. Команда PasteWidgetCommand затем читает эти
// свойства и создаёт точную копию виджета в нужном месте.
//
// WidgetClipboard живёт в единственном экземпляре на всё приложение
// и хранит:
//   - тип скопированного виджета (Button, Label, Panel…)
//   - карту свойств: имя → значение (текст, ширина, цвет, шрифт…)
//   - флаг, есть ли вообще что-то в буфере
//
// Буфер не зависит от платформы и не взаимодействует с системным
// буфером обмена macOS/Windows. Это сделано специально, чтобы
// копирование/вставка работали одинаково на всех ОС и не зависели
// от прав доступа или форматов системного буфера. В будущем можно
// добавить мост к системному буферу через PlatformAdapter.
//
// Как использовать:
//   1. При копировании: WidgetClipboard::instance().setContent(...)
//   2. При вставке:     if (WidgetClipboard::instance().hasContent()) { ... }
//   3. При создании нового документа буфер очищается.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/WidgetType.hpp"
#include "../../Core/State/StateValue.hpp"
#include <unordered_map>
#include <string>

namespace MirUI {

class WidgetClipboard {
public:
    // ── Доступ к синглтону ──────────────────────────────────
    // Единственный экземпляр на всё приложение.
    static WidgetClipboard& instance() {
        static WidgetClipboard s_clipboard;
        return s_clipboard;
    }

    // ── Сохранить содержимое в буфер ────────────────────────
    // Принимает тип виджета и карту свойств (имя → значение).
    // Заменяет предыдущее содержимое буфера.
    void setContent(WidgetType type, std::unordered_map<std::string, StateValue> properties) {
        m_type = type;
        m_properties = std::move(properties);
        m_hasContent = true;
    }

    // ── Проверить, есть ли что-то в буфере ──────────────────
    [[nodiscard]] bool hasContent() const {
        return m_hasContent;
    }

    // ── Получить тип скопированного виджета ─────────────────
    [[nodiscard]] WidgetType type() const {
        return m_type;
    }

    // ── Получить карту свойств (только для чтения) ──────────
    [[nodiscard]] const std::unordered_map<std::string, StateValue>& properties() const {
        return m_properties;
    }

    // ── Очистить буфер ──────────────────────────────────────
    // Вызывается, например, при создании нового документа.
    void clear() {
        m_type = WidgetType::Unknown;
        m_properties.clear();
        m_hasContent = false;
    }

private:
    // Приватный конструктор (синглтон).
    WidgetClipboard() = default;

    WidgetType m_type = WidgetType::Unknown;
    std::unordered_map<std::string, StateValue> m_properties;
    bool m_hasContent = false;
};

} // namespace MirUI