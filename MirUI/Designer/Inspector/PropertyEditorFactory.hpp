
#pragma once

#include "PropertyEditor.hpp"
#include "InspectorModel.hpp"
#include "../Document/UIDocument.hpp"
#include <memory>

namespace MirUI {

class PropertyEditorFactory {
public:

    static std::unique_ptr<PropertyEditor> create(UIDocument& doc,
                                                  WidgetID widgetId,
                                                  const PropertyEntry& entry) {

        return std::make_unique<PropertyEditor>(doc, widgetId, entry);
    }
};

}