
#pragma once

#include "WidgetID.hpp"
#include "WidgetType.hpp"
#include "../Layout/Rect.hpp"
#include "../Layout/LayoutData.hpp"
#include "../../Core/State/StateValue.hpp"
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>

namespace MirUI {

class Widget {
public:
    explicit Widget(WidgetType type = WidgetType::Unknown);
    virtual ~Widget();

    [[nodiscard]] WidgetID id()   const noexcept;
    [[nodiscard]] WidgetType type() const noexcept;

    [[nodiscard]] std::string name() const;
    void setName(const std::string& newName);

    void setVisible(bool visible);
    [[nodiscard]] bool isVisible() const;
    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const;

    void setBounds(const Rect& bounds) noexcept;
    [[nodiscard]] const Rect& bounds() const noexcept;

    void setLayoutData(const LayoutData& ld);
    [[nodiscard]] const LayoutData& layoutData() const;
    [[nodiscard]] LayoutData& layoutData();

    [[nodiscard]] Widget* parent() const noexcept;
    [[nodiscard]] const std::vector<Widget*>& children() const noexcept;

    void addChild(Widget* child);
    bool removeChild(WidgetID id);

    void requestFocus() noexcept;
    [[nodiscard]] bool hasFocus() const noexcept;
    void setFocusInternal(bool focused) noexcept;

    virtual bool setProperty(const std::string& propertyName, const StateValue& value);
    virtual std::optional<StateValue> getProperty(const std::string& propertyName) const;

    [[nodiscard]] const std::unordered_map<std::string, StateValue>& allProperties() const;

protected:
    WidgetID m_id;
    WidgetType m_type;
    Widget* m_parent = nullptr;
    std::vector<Widget*> m_children;
    bool m_visible;
    bool m_enabled;
    bool m_focused;
    Rect m_bounds{0, 0, 0, 0};
    LayoutData m_layoutData;
    std::unordered_map<std::string, StateValue> m_properties;
};

}