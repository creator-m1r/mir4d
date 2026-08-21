
#import <Foundation/Foundation.h>
#include "SwiftUIRenderer.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Theme/ThemeResolver.hpp"
#include <cstring>
#include <sstream>
#include <iomanip>

namespace MirUI {

SwiftUIRenderer::SwiftUIRenderer()
    : m_context(nullptr), m_currentParentIndex(-1), m_stackDepth(0) {}
SwiftUIRenderer::~SwiftUIRenderer() = default;

void SwiftUIRenderer::beginFrame() {
    m_viewNodes.clear();
    m_currentParentIndex = -1;
    m_stackDepth = 0;
}

void SwiftUIRenderer::requestUpdate(int64_t widgetId) {
    (void)widgetId;
}

void SwiftUIRenderer::endFrame() {
    flushToSwift();
}

void SwiftUIRenderer::applyWidgetStyle(SwiftUIViewNode& node, WidgetType type, WidgetState state) {
    if (!m_context) return;

    Theme currentTheme = m_context->themeManager().current();
    ThemeResolver resolver(currentTheme);

    WidgetStyle style = resolver.resolve(type, state);

    node.backgroundHex   = colorToHex(style.background);
    node.foregroundHex   = colorToHex(style.foreground);
    node.borderHex       = colorToHex(style.border);
    node.cornerRadius    = style.cornerRadius;
    node.fontFamily      = style.font.family;
    node.fontSize        = style.font.size;
    node.fontWeight      = fontWeightToInt(style.font.weight);
    node.fontStyle       = (style.font.style == FontStyle::Italic) ? 1 : 0;
    node.opacity         = style.opacity;
    node.visible         = style.visible;

    node.shadowOffsetX   = style.shadow.offsetX;
    node.shadowOffsetY   = style.shadow.offsetY;
    node.shadowBlur      = style.shadow.blurRadius;
    node.shadowColorHex  = colorToHex(style.shadow.color);
}

void SwiftUIRenderer::renderButton(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Button";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("text");
    if (p && std::holds_alternative<std::string>(*p)) node.text = std::get<std::string>(*p);
    p = widget.getProperty("command");
    if (p && std::holds_alternative<std::string>(*p)) node.commandId = std::get<std::string>(*p);
    applyWidgetStyle(node, WidgetType::Button, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderLabel(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Label";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("text");
    if (p && std::holds_alternative<std::string>(*p)) node.text = std::get<std::string>(*p);
    else node.text = widget.name();
    applyWidgetStyle(node, WidgetType::Label, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderCheckBox(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "CheckBox";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("text");
    if (p && std::holds_alternative<std::string>(*p)) node.text = std::get<std::string>(*p);
    p = widget.getProperty("checked");
    if (p && std::holds_alternative<bool>(*p)) node.checked = std::get<bool>(*p);
    applyWidgetStyle(node, WidgetType::CheckBox, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderTextField(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "TextField";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("text");
    if (p && std::holds_alternative<std::string>(*p)) node.text = std::get<std::string>(*p);
    p = widget.getProperty("placeholder");
    if (p && std::holds_alternative<std::string>(*p)) node.placeholder = std::get<std::string>(*p);
    applyWidgetStyle(node, WidgetType::TextField, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderComboBox(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "ComboBox";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("items");
    if (p && std::holds_alternative<std::string>(*p)) node.items = std::get<std::string>(*p);
    p = widget.getProperty("selectedIndex");
    if (p && std::holds_alternative<int64_t>(*p)) node.selectedIndex = static_cast<int>(std::get<int64_t>(*p));
    applyWidgetStyle(node, WidgetType::ComboBox, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderSlider(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Slider";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("value");
    if (p && std::holds_alternative<double>(*p)) node.value = std::get<double>(*p);
    p = widget.getProperty("minValue"); node.minValue = (p && std::holds_alternative<double>(*p)) ? std::get<double>(*p) : 0.0;
    p = widget.getProperty("maxValue"); node.maxValue = (p && std::holds_alternative<double>(*p)) ? std::get<double>(*p) : 100.0;
    applyWidgetStyle(node, WidgetType::Slider, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderRadioButton(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "RadioButton";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("text");
    if (p && std::holds_alternative<std::string>(*p)) node.text = std::get<std::string>(*p);
    p = widget.getProperty("checked");
    if (p && std::holds_alternative<bool>(*p)) node.checked = std::get<bool>(*p);
    p = widget.getProperty("groupId");
    if (p && std::holds_alternative<std::string>(*p)) node.groupId = std::get<std::string>(*p);
    applyWidgetStyle(node, WidgetType::RadioButton, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderProgressBar(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "ProgressBar";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("value");
    if (p && std::holds_alternative<double>(*p)) node.value = std::get<double>(*p);
    p = widget.getProperty("minValue"); node.minValue = (p && std::holds_alternative<double>(*p)) ? std::get<double>(*p) : 0.0;
    p = widget.getProperty("maxValue"); node.maxValue = (p && std::holds_alternative<double>(*p)) ? std::get<double>(*p) : 100.0;
    p = widget.getProperty("indeterminate");
    if (p && std::holds_alternative<bool>(*p)) node.indeterminate = std::get<bool>(*p);
    applyWidgetStyle(node, WidgetType::ProgressBar, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderImage(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Image";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("source");
    if (p && std::holds_alternative<std::string>(*p)) node.source = std::get<std::string>(*p);
    p = widget.getProperty("fit");
    if (p && std::holds_alternative<std::string>(*p)) node.fit = std::get<std::string>(*p);
    p = widget.getProperty("opacity");
    if (p && std::holds_alternative<double>(*p)) node.opacity = std::get<double>(*p);
    applyWidgetStyle(node, WidgetType::Image, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderTableView(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "TableView";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("columns");
    if (p && std::holds_alternative<std::string>(*p)) node.columns = std::get<std::string>(*p);
    p = widget.getProperty("data");
    if (p && std::holds_alternative<std::string>(*p)) node.data = std::get<std::string>(*p);
    p = widget.getProperty("selectedRow");
    if (p && std::holds_alternative<int64_t>(*p)) node.selectedRow = std::get<int64_t>(*p);
    applyWidgetStyle(node, WidgetType::TableView, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderContainer(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Container";
    fillBaseNode(node, widget);
    node.text = widget.name();
    applyWidgetStyle(node, WidgetType::Panel, WidgetState::Normal);
    int idx = addViewNode(std::move(node));
    pushParent(idx);
}

void SwiftUIRenderer::renderToolbar(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Toolbar";
    fillBaseNode(node, widget);
    node.text = widget.name();
    applyWidgetStyle(node, WidgetType::Toolbar, WidgetState::Normal);
    int idx = addViewNode(std::move(node));
    pushParent(idx);
}

void SwiftUIRenderer::renderScrollView(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "ScrollView";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("scrollDirection");
    if (p && std::holds_alternative<std::string>(*p)) node.scrollDirection = std::get<std::string>(*p);
    p = widget.getProperty("showScrollBars");
    if (p && std::holds_alternative<bool>(*p)) node.showScrollBars = std::get<bool>(*p);
    applyWidgetStyle(node, WidgetType::ScrollView, WidgetState::Normal);
    int idx = addViewNode(std::move(node));
    pushParent(idx);
}

void SwiftUIRenderer::renderTabView(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "TabView";
    fillBaseNode(node, widget);
    auto p = widget.getProperty("tabTitles");
    if (p && std::holds_alternative<std::string>(*p)) node.tabTitles = std::get<std::string>(*p);
    p = widget.getProperty("selectedTab");
    if (p && std::holds_alternative<int64_t>(*p)) node.selectedTab = std::get<int64_t>(*p);
    p = widget.getProperty("tabPosition");
    if (p && std::holds_alternative<std::string>(*p)) node.tabPosition = std::get<std::string>(*p);
    applyWidgetStyle(node, WidgetType::TabView, WidgetState::Normal);
    int idx = addViewNode(std::move(node));
    pushParent(idx);
}

void SwiftUIRenderer::renderTree(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Tree";
    fillBaseNode(node, widget);
    applyWidgetStyle(node, WidgetType::Tree, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderPropertyGrid(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "PropertyGrid";
    fillBaseNode(node, widget);
    applyWidgetStyle(node, WidgetType::PropertyGrid, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderViewport(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Viewport";
    fillBaseNode(node, widget);
    applyWidgetStyle(node, WidgetType::Viewport, WidgetState::Normal);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::renderUnknownWidget(Widget& widget) {
    SwiftUIViewNode node;
    node.type = "Unknown";
    fillBaseNode(node, widget);
    addViewNode(std::move(node));
}

void SwiftUIRenderer::fillBaseNode(SwiftUIViewNode& node, Widget& widget) {
    node.widgetId = static_cast<int64_t>(widget.id().value());
    Rect bounds = widget.bounds();
    node.x = bounds.x;
    node.y = bounds.y;
    node.width = bounds.width;
    node.height = bounds.height;
    node.visible = widget.isVisible();
    node.parentIndex = m_currentParentIndex;
}

void SwiftUIRenderer::pushParent(int nodeIndex) {
    if (m_stackDepth < 64) {
        m_parentIndexStack[m_stackDepth++] = m_currentParentIndex;
    }
    m_currentParentIndex = nodeIndex;
}

void SwiftUIRenderer::popParent() {
    if (m_stackDepth > 0) {
        m_currentParentIndex = m_parentIndexStack[--m_stackDepth];
    } else {
        m_currentParentIndex = -1;
    }
}

int SwiftUIRenderer::addViewNode(SwiftUIViewNode node) {
    int index = static_cast<int>(m_viewNodes.size());
    m_viewNodes.push_back(std::move(node));
    return index;
}

std::string SwiftUIRenderer::colorToHex(const Color& color) {
    std::ostringstream oss;
    oss << "#"
        << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(color.r * 255)
        << std::setw(2) << static_cast<int>(color.g * 255)
        << std::setw(2) << static_cast<int>(color.b * 255)
        << std::setw(2) << static_cast<int>(color.a * 255);
    return oss.str();
}

int SwiftUIRenderer::fontWeightToInt(FontWeight weight) {
    switch (weight) {
        case FontWeight::Thin:       return 100;
        case FontWeight::ExtraLight: return 200;
        case FontWeight::Light:      return 300;
        case FontWeight::Regular:    return 400;
        case FontWeight::Medium:     return 500;
        case FontWeight::SemiBold:   return 600;
        case FontWeight::Bold:       return 700;
        case FontWeight::ExtraBold:  return 800;
        case FontWeight::Black:      return 900;
    }
    return 400;
}

void SwiftUIRenderer::flushToSwift() {
    if (m_viewNodes.empty()) return;
    NSLog(@"[MirUI SwiftUI Renderer] Передаём %lu узлов в SwiftUI", (unsigned long)m_viewNodes.size());

}

extern "C" void MirUI_SwiftUI_UpdateViewNodes(const SwiftUIViewNode* nodes, int count, int rootIndex) {
    NSLog(@"[Swift] Получено %d узлов для отображения (корень=%d)", count, rootIndex);
    (void)nodes;
}

}