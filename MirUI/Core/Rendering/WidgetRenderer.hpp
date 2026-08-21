// MirUI/Core/Rendering/WidgetRenderer.hpp
// 🖼️ Базовый рендерер виджетов — обходит дерево и вызывает методы для каждого типа.
//
// У нас уже есть абстрактный Renderer (Core/Rendering/Renderer.hpp), который
// объявляет единственный метод render(WidgetTree&). Этого достаточно для
// самых простых случаев, но когда мы начинаем писать адаптеры под SwiftUI,
// WinUI или WebUI, удобнее иметь отдельный виртуальный метод для каждого
// типа виджета: renderButton, renderLabel, renderContainer, renderTree и так далее.
//
// WidgetRenderer — это промежуточный слой. Он реализует обход дерева
// в методе render(), а для каждого встреченного виджета вызывает
// соответствующий защищённый виртуальный метод. Конкретный адаптер
// (например, SwiftUIRenderer) просто переопределяет эти методы
// и создаёт внутри нативные представления.
//
// Такой подход даёт три больших плюса:
//   1. Код адаптера становится чистым: он занимается только отрисовкой,
//      а не обходом дерева.
//   2. Если мы добавим новый тип виджета в Core, мы просто добавим новый
//      виртуальный метод сюда и во все адаптеры. Компилятор подскажет,
//      где нужно дописать реализацию.
//   3. Базовый render() умеет правильно обрабатывать контейнеры и вложенность,
//      автоматически заходя в детей после отрисовки родителя.
//
// Как происходит отрисовка (упрощённо):
//   render(tree) обходит дерево в глубину.
//   Для каждого виджета смотрит на WidgetType и вызывает нужный метод:
//     Button       → renderButton()
//     Label        → renderLabel()
//     Container    → renderContainer()
//     Tree         → renderTree()
//     PropertyGrid → renderPropertyGrid()
//     Viewport     → renderViewport()
//     ...          → renderUnknownWidget()
//   После вызова метода рендерер автоматически заходит в детей
//   (если это не листовой виджет) и вызывает для них те же шаги.
//
// Важно: WidgetRenderer сам ничего не рисует — он только организует
// процесс. Вся графика остаётся в наследниках.
//
// Чистый C++23, без платформенных зависимостей.
// MirUI/Core/Rendering/WidgetRenderer.hpp — обновлённый с методами для новых виджетов.

#pragma once

#include "../Widget/WidgetTree.hpp"
#include "../Widget/WidgetType.hpp"
#include "Renderer.hpp"

namespace MirUI {

class WidgetRenderer : public Renderer {
public:
    virtual ~WidgetRenderer() = default;

    void render(WidgetTree& tree) override {
        Widget* root = tree.root();
        if (root) {
            renderWidgetRecursive(root);
        }
    }

protected:
    virtual void renderButton(Widget& widget)       { (void)widget; }
    virtual void renderLabel(Widget& widget)        { (void)widget; }
    virtual void renderToolbar(Widget& widget)      { (void)widget; }
    virtual void renderTree(Widget& widget)         { (void)widget; }
    virtual void renderPropertyGrid(Widget& widget) { (void)widget; }
    virtual void renderViewport(Widget& widget)     { (void)widget; }
    virtual void renderContainer(Widget& widget)    { (void)widget; }
    virtual void renderRibbon(Widget& widget)       { (void)widget; }
    virtual void renderTimeline(Widget& widget)     { (void)widget; }
    virtual void renderUnknownWidget(Widget& widget) { (void)widget; }

    // Новые методы для виджетов форм
    virtual void renderCheckBox(Widget& widget)     { (void)widget; }
    virtual void renderTextField(Widget& widget)    { (void)widget; }
    virtual void renderComboBox(Widget& widget)     { (void)widget; }
    virtual void renderSlider(Widget& widget)       { (void)widget; }

    virtual void beginContainer(Widget& /*container*/) {}
    virtual void endContainer(Widget& /*container*/) {}

private:
    void renderWidgetRecursive(Widget* widget) {
        if (!widget || !widget->isVisible()) return;

        switch (widget->type()) {
            case WidgetType::Button:       renderButton(*widget); break;
            case WidgetType::Label:        renderLabel(*widget); break;
            case WidgetType::Toolbar:      renderToolbar(*widget); break;
            case WidgetType::Tree:         renderTree(*widget); break;
            case WidgetType::PropertyGrid: renderPropertyGrid(*widget); break;
            case WidgetType::Viewport:     renderViewport(*widget); break;
            case WidgetType::Ribbon:       renderRibbon(*widget); break;
            case WidgetType::Timeline:     renderTimeline(*widget); break;
            case WidgetType::CheckBox:     renderCheckBox(*widget); break;
            case WidgetType::TextField:    renderTextField(*widget); break;
            case WidgetType::ComboBox:     renderComboBox(*widget); break;
            case WidgetType::Slider:       renderSlider(*widget); break;
            case WidgetType::Panel:
            case WidgetType::DockPanel:
            case WidgetType::Window:
                renderContainer(*widget);
                break;
            default:
                renderUnknownWidget(*widget);
                break;
        }

        if (!widget->children().empty()) {
            beginContainer(*widget);
            for (Widget* child : widget->children()) {
                renderWidgetRecursive(child);
            }
            endContainer(*widget);
        }
    }
};

} // namespace MirUI