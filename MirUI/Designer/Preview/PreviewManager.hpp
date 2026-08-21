// MirUI/Designer/Preview/PreviewManager.hpp
// 👁️ Менеджер режима предпросмотра в MirUI Designer.
//
// Обычно холст редактора показывает вспомогательные элементы:
// ручки изменения размера, направляющие линии, сетку, рамки выделения.
// Когда пользователь хочет посмотреть, как интерфейс будет выглядеть
// в реальном приложении, он нажимает кнопку «Preview» (Предпросмотр).
//
// PreviewManager управляет этим режимом:
//   • Хранит флаг, включён ли предпросмотр.
//   • При входе в предпросмотр отключает все вспомогательные элементы
//     (направляющие, сетку, ручки) и может временно скрыть панели редактора.
//   • При выходе восстанавливает исходное состояние.
//   • Уведомляет другие компоненты (холст, инспектор) об изменении режима
//     через простые функции-колбэки, чтобы они могли обновить отображение.
//
// Важно: PreviewManager не меняет сам интерфейс (дерево виджетов) —
// он только управляет визуальными настройками редактора.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace MirUI {

// Вперёд объявим классы, которыми мы управляем, чтобы избежать циклических зависимостей.
class GridManager;
class GuideManager;

class PreviewManager {
public:
    // ── Конструктор ──────────────────────────────────────────
    PreviewManager()
        : m_previewMode(false)
    {}

    // ── Вход в режим предпросмотра ───────────────────────────
    // Отключает сетку, направляющие и вызывает все зарегистрированные колбэки.
    void enterPreview() {
        if (m_previewMode) return; // уже в предпросмотре
        m_previewMode = true;

        // Отключаем вспомогательные элементы.
        setGridVisible(false);
        setGuidesVisible(false);

        // Вызываем пользовательские колбэки (например, скрыть панель инструментов).
        for (auto& callback : m_onEnterCallbacks) {
            if (callback) callback();
        }
    }

    // ── Выход из режима предпросмотра ────────────────────────
    // Восстанавливает настройки (сетка, направляющие) и вызывает колбэки.
    void exitPreview() {
        if (!m_previewMode) return;
        m_previewMode = false;

        // Восстанавливаем видимость (включаем то, что было отключено).
        setGridVisible(true);
        setGuidesVisible(true);

        for (auto& callback : m_onExitCallbacks) {
            if (callback) callback();
        }
    }

    // ── Переключение режима ──────────────────────────────────
    void togglePreview() {
        if (m_previewMode) {
            exitPreview();
        } else {
            enterPreview();
        }
    }

    // ── Проверка состояния ───────────────────────────────────
    [[nodiscard]] bool isPreviewMode() const { return m_previewMode; }

    // ── Регистрация колбэков ─────────────────────────────────
    // Позволяет другим компонентам (холст, тулбокс) реагировать на смену режима.
    // Например, DesignerCanvas может заблокировать выделение и перетаскивание.
    void onEnter(const std::function<void()>& callback) {
        m_onEnterCallbacks.push_back(callback);
    }
    void onExit(const std::function<void()>& callback) {
        m_onExitCallbacks.push_back(callback);
    }

    // ── Прямое управление менеджерами (если они доступны) ────
    // Эти методы сохраняют состояние перед входом в Preview, чтобы восстановить его.
    void attachGridManager(GridManager* grid) { m_grid = grid; }
    void attachGuideManager(GuideManager* guide) { m_guide = guide; }

private:
    bool m_previewMode;

    // Указатели на управляемые сервисы (могут быть nullptr).
    GridManager* m_grid = nullptr;
    GuideManager* m_guide = nullptr;

    // Сохранённые состояния перед предпросмотром (чтобы восстановить).
    bool m_savedGridVisible = true;
    bool m_savedGuidesVisible = true;

    // Колбэки для уведомления других частей редактора.
    std::vector<std::function<void()>> m_onEnterCallbacks;
    std::vector<std::function<void()>> m_onExitCallbacks;

    // Вспомогательные методы для включения/выключения.
    void setGridVisible(bool visible) {
        if (m_grid) {
            if (visible) {
                m_grid->setVisible(m_savedGridVisible);
            } else {
                m_savedGridVisible = m_grid->isVisible();
                m_grid->setVisible(false);
            }
        }
    }

    void setGuidesVisible(bool visible) {
        if (m_guide) {
            if (visible) {
                m_guide->setVisible(m_savedGuidesVisible);
            } else {
                m_savedGuidesVisible = m_guide->isVisible();
                m_guide->setVisible(false);
            }
        }
    }
};

} // namespace MirUI