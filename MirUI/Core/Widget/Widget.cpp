// MirUI/Core/Widget/Widget.cpp
// 🧱 Реализация базового класса Widget.
// Содержит не-inline методы, объявленные в Widget.hpp.
// Это позволяет держать заголовок компактным, а реализацию —
// в отдельной единице трансляции, что ускоряет сборку при изменениях.
//
// Чистый C++23, без платформенных зависимостей.

// MirUI/Core/Widget/Widget.cpp
// 🧱 Реализация базового класса Widget.
// Весь код, который ранее был inline, теперь находится здесь.
// Чистый C++23, без платформенных зависимостей.

#include "Widget.hpp"
#include <algorithm>

namespace MirUI {

// ── Конструктор ──────────────────────────────────────────────
Widget::Widget(WidgetType type)
    : m_id(WidgetID::generate())
    , m_type(type)
    , m_parent(nullptr)
    , m_visible(true)
    , m_enabled(true)
    , m_focused(false)
    , m_layoutData(LayoutData::fit())
{
    m_properties["visible"] = StateValue(true);
    m_properties["enabled"] = StateValue(true);
    m_properties["name"]    = StateValue(std::string(""));
}

// ── Деструктор ───────────────────────────────────────────────
Widget::~Widget() {
    for (Widget* child : m_children) {
        delete child;
    }
}

// ── Идентификация ────────────────────────────────────────────
WidgetID Widget::id() const noexcept {
    return m_id;
}

WidgetType Widget::type() const noexcept {
    return m_type;
}

// ── Имя ──────────────────────────────────────────────────────
std::string Widget::name() const {
    auto it = m_properties.find("name");
    if (it != m_properties.end() && std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
    }
    return "";
}

void Widget::setName(const std::string& newName) {
    m_properties["name"] = StateValue(newName);
}

// ── Видимость и доступность ──────────────────────────────────
void Widget::setVisible(bool visible) {
    m_properties["visible"] = StateValue(visible);
}

bool Widget::isVisible() const {
    auto it = m_properties.find("visible");
    return (it != m_properties.end() && std::holds_alternative<bool>(it->second))
               ? std::get<bool>(it->second)
               : true;
}

void Widget::setEnabled(bool enabled) {
    m_properties["enabled"] = StateValue(enabled);
}

bool Widget::isEnabled() const {
    auto it = m_properties.find("enabled");
    return (it != m_properties.end() && std::holds_alternative<bool>(it->second))
               ? std::get<bool>(it->second)
               : true;
}

// ── Геометрия ────────────────────────────────────────────────
void Widget::setBounds(const Rect& bounds) noexcept {
    m_bounds = bounds;
}

const Rect& Widget::bounds() const noexcept {
    return m_bounds;
}

// ── Данные компоновки ───────────────────────────────────────
void Widget::setLayoutData(const LayoutData& ld) {
    m_layoutData = ld;
}

const LayoutData& Widget::layoutData() const {
    return m_layoutData;
}

LayoutData& Widget::layoutData() {
    return m_layoutData;
}

// ── Дерево ───────────────────────────────────────────────────
Widget* Widget::parent() const noexcept {
    return m_parent;
}

const std::vector<Widget*>& Widget::children() const noexcept {
    return m_children;
}

void Widget::addChild(Widget* child) {
    if (!child) return;
    if (child->m_parent) {
        child->m_parent->removeChild(child->m_id);
    }
    child->m_parent = this;
    m_children.push_back(child);
}

bool Widget::removeChild(WidgetID id) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
        [id](const Widget* w) { return w->id() == id; });
    if (it != m_children.end()) {
        Widget* child = *it;
        m_children.erase(it);
        child->m_parent = nullptr;
        return true;
    }
    return false;
}

// ── Фокус ────────────────────────────────────────────────────
void Widget::requestFocus() noexcept {
    m_focused = true;
}

bool Widget::hasFocus() const noexcept {
    return m_focused;
}

void Widget::setFocusInternal(bool focused) noexcept {
    m_focused = focused;
}

// ── Универсальные свойства ──────────────────────────────────
bool Widget::setProperty(const std::string& propertyName, const StateValue& value) {
    if (propertyName == "visible") {
        if (std::holds_alternative<bool>(value)) {
            setVisible(std::get<bool>(value));
            return true;
        }
        return false;
    }
    if (propertyName == "enabled") {
        if (std::holds_alternative<bool>(value)) {
            setEnabled(std::get<bool>(value));
            return true;
        }
        return false;
    }
    if (propertyName == "name") {
        if (std::holds_alternative<std::string>(value)) {
            setName(std::get<std::string>(value));
            return true;
        }
        return false;
    }
    if (propertyName == "width") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.widthValue = std::get<double>(value);
            m_layoutData.widthUnit  = Unit::Pixel;
            return true;
        }
        return false;
    }
    if (propertyName == "height") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.heightValue = std::get<double>(value);
            m_layoutData.heightUnit  = Unit::Pixel;
            return true;
        }
        return false;
    }
    if (propertyName == "minWidth") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.minimumSize.width = std::get<double>(value);
            return true;
        }
        return false;
    }
    if (propertyName == "minHeight") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.minimumSize.height = std::get<double>(value);
            return true;
        }
        return false;
    }
    if (propertyName == "maxWidth") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.maximumSize.width = std::get<double>(value);
            return true;
        }
        return false;
    }
    if (propertyName == "maxHeight") {
        if (std::holds_alternative<double>(value)) {
            m_layoutData.maximumSize.height = std::get<double>(value);
            return true;
        }
        return false;
    }

    m_properties[propertyName] = value;
    return true;
}

std::optional<StateValue> Widget::getProperty(const std::string& propertyName) const {
    if (propertyName == "visible")  return StateValue(isVisible());
    if (propertyName == "enabled")  return StateValue(isEnabled());
    if (propertyName == "name")     return StateValue(this->name());   // исправлено
    if (propertyName == "width")    return StateValue(m_layoutData.widthValue);
    if (propertyName == "height")   return StateValue(m_layoutData.heightValue);
    if (propertyName == "minWidth") return StateValue(m_layoutData.minimumSize.width);
    if (propertyName == "minHeight") return StateValue(m_layoutData.minimumSize.height);
    if (propertyName == "maxWidth") return StateValue(m_layoutData.maximumSize.width);
    if (propertyName == "maxHeight") return StateValue(m_layoutData.maximumSize.height);

    auto it = m_properties.find(propertyName);
    if (it != m_properties.end()) {
        return it->second;
    }
    return std::nullopt;
}

const std::unordered_map<std::string, StateValue>& Widget::allProperties() const {
    return m_properties;
}

} // namespace MirUI