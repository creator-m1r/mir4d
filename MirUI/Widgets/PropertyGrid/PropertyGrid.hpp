
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "Property.hpp"
#include <vector>
#include <optional>
#include <algorithm>
#include <string>

namespace MirUI {

class PropertyGrid : public Widget {
public:
    PropertyGrid()
        : Widget(WidgetType::PropertyGrid)
    {}

    void addProperty(const Property& property) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == property.id; });
        if (it != m_properties.end()) {
            *it = property;
        } else {
            m_properties.push_back(property);
        }
    }

    void setProperties(const std::vector<Property>& properties) {
        m_properties = properties;
    }

    void clear() noexcept {
        m_properties.clear();
    }

    bool removeProperty(const std::string& id) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            m_properties.erase(it);
            return true;
        }
        return false;
    }

    [[nodiscard]] const std::vector<Property>& properties() const {
        return m_properties;
    }

    bool setValue(const std::string& id, const StateValue& newValue) {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            if (it->readOnly) {
                return false;
            }
            it->value = newValue;
            return true;
        }
        return false;
    }

    [[nodiscard]] std::optional<StateValue> value(const std::string& id) const {
        auto it = std::find_if(m_properties.begin(), m_properties.end(),
            [&](const Property& p) { return p.id == id; });
        if (it != m_properties.end()) {
            return it->value;
        }
        return std::nullopt;
    }

private:
    std::vector<Property> m_properties;
};

}
