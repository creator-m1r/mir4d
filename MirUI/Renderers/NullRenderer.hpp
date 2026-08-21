// MirUI/Renderers/NullRenderer.hpp
// Заглушка рендерера для тестирования и отладки ядра MirUI.
// Ничего не рисует на экране, но проходит по всему дереву виджетов
// и для каждого виджета выводит информацию в консоль (или просто считает их).
// Это позволяет проверить, что все виджеты созданы, дерево построено правильно,
// LayoutEngine отработал, а команды рендеринга сформированы.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Core/Rendering/Renderer.hpp"
#include "../Core/Widget/WidgetTree.hpp"
#include "../Core/Widget/Widget.hpp"
#include <iostream>
#include <string>

namespace MirUI {

class NullRenderer : public Renderer {
public:
    // ── Реализация абстрактного интерфейса Renderer ──────────

    void beginFrame() override {
        // В начале каждого кадра сбрасываем счётчики.
        m_widgetCount = 0;
        std::cout << "=== NullRenderer: начало кадра ===" << std::endl;
    }

    void render(WidgetTree& tree) override {
        // Запускаем обход дерева с корневого виджета.
        Widget* root = tree.root();
        if (root) {
            std::cout << "Начинаем обход дерева виджетов..." << std::endl;
            renderWidgetRecursive(root, 0);
        } else {
            std::cout << "Дерево виджетов пустое — нечего рисовать." << std::endl;
        }
    }

    void endFrame() override {
        std::cout << "=== NullRenderer: конец кадра ===" << std::endl;
        std::cout << "Всего виджетов обработано: " << m_widgetCount << std::endl << std::endl;
    }

    // ── Дополнительный метод для получения количества виджетов ──
    [[nodiscard]] int getWidgetCount() const { return m_widgetCount; }

private:
    int m_widgetCount = 0; // Счётчик обработанных виджетов за кадр.

    // Рекурсивная функция обхода: печатает отступы и информацию о виджете.
    void renderWidgetRecursive(Widget* widget, int depth) {
        if (!widget) return;

        // Увеличиваем счётчик
        ++m_widgetCount;

        // Создаём отступ для красивого вывода (как в дереве папок).
        std::string indent(depth * 2, ' ');

        // Выводим базовую информацию о виджете.
        std::cout << indent
                  << "Виджет ID=" << widget->id().value()
                  << " | тип: " << widgetTypeToString(widget->type())
                  << " | visible=" << (widget->isVisible() ? "да" : "нет")
                  << " | bounds=(" << widget->bounds().x << ", " << widget->bounds().y
                  << ", " << widget->bounds().width << "x" << widget->bounds().height << ")"
                  << std::endl;

        // Рекурсивно обходим всех детей.
        for (Widget* child : widget->children()) {
            renderWidgetRecursive(child, depth + 1);
        }
    }

    // Преобразование типа виджета в читаемую строку (для вывода).
    static std::string widgetTypeToString(WidgetType type) {
        switch (type) {
            case WidgetType::Unknown:      return "Unknown";
            case WidgetType::Window:       return "Window";
            case WidgetType::Panel:        return "Panel";
            case WidgetType::Button:       return "Button";
            case WidgetType::Label:        return "Label";
            case WidgetType::Tree:         return "Tree";
            case WidgetType::PropertyGrid: return "PropertyGrid";
            case WidgetType::Ribbon:       return "Ribbon";
            case WidgetType::Toolbar:      return "Toolbar";
            case WidgetType::DockPanel:    return "DockPanel";
            case WidgetType::Viewport:     return "Viewport";
            case WidgetType::Timeline:     return "Timeline";
            default:                       return "???";
        }
    }
};

} // namespace MirUI