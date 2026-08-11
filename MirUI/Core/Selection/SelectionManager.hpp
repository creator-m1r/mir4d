// MirUI/Core/Selection/SelectionManager.hpp
// 🖱️ Менеджер выделения — следит за тем, какие виджеты сейчас выбраны.
// Позволяет выделять один или несколько виджетов, снимать выделение,
// проверять, выделен ли конкретный виджет, и очищать всё выделение.
// Используется как в редакторе (Designer), так и в рантайме (например,
// для PropertyGrid, который показывает свойства выделенного объекта).
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Widget/WidgetID.hpp"
#include <vector>
#include <algorithm>

namespace MirUI {

class SelectionManager {
public:
    // ── Выделение одного виджета ────────────────────────────
    // Если нужно выделить только один виджет (например, кликнули мышкой),
    // то сначала очищаем старое выделение, а затем выделяем новый.
    void select(WidgetID id) {
        // Если этот виджет уже выделен и он единственный — ничего не делаем.
        if (m_selected.size() == 1 && m_selected[0] == id) {
            return;
        }
        // Очищаем предыдущее выделение и добавляем новый.
        m_selected.clear();
        m_selected.push_back(id);
    }

    // ── Множественное выделение (добавить к уже выделенным) ─
    // Используется, например, при выделении с зажатым Ctrl.
    void addToSelection(WidgetID id) {
        // Если id уже есть в списке — не добавляем дубликат.
        if (isSelected(id)) {
            return;
        }
        m_selected.push_back(id);
    }

    // ── Снятие выделения с одного виджета ───────────────────
    void deselect(WidgetID id) {
        // Удаляем id из вектора (порядок остальных сохраняется).
        auto it = std::remove(m_selected.begin(), m_selected.end(), id);
        m_selected.erase(it, m_selected.end());
    }

    // ── Полная очистка выделения ────────────────────────────
    void clear() {
        m_selected.clear();
    }

    // ── Проверка, выделен ли виджет ─────────────────────────
    [[nodiscard]] bool isSelected(WidgetID id) const {
        return std::find(m_selected.begin(), m_selected.end(), id) != m_selected.end();
    }

    // ── Получить список всех выделенных виджетов ────────────
    // Возвращает константную ссылку на вектор (не копию).
    [[nodiscard]] const std::vector<WidgetID>& selected() const {
        return m_selected;
    }

    // ── Удобные методы для работы с одним выделенным виджетом ──
    // Если выделен ровно один виджет — вернуть его ID, иначе std::nullopt.
    [[nodiscard]] std::optional<WidgetID> singleSelected() const {
        if (m_selected.size() == 1) {
            return m_selected[0];
        }
        return std::nullopt;
    }

    // Количество выделенных виджетов.
    [[nodiscard]] size_t count() const {
        return m_selected.size();
    }

    // Есть ли вообще выделенные виджеты?
    [[nodiscard]] bool empty() const {
        return m_selected.empty();
    }

private:
    std::vector<WidgetID> m_selected; // Список ID выделенных виджетов.
};

} // namespace MirUI