
#pragma once

#include "../Model/UIProject.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"
#include "../../Core/Layout/LayoutEngine.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>

namespace MirUI {

class UIProjectSerializer {
public:

    bool save(const std::string& path, UIProject& project) {
        std::ofstream file(path);
        if (!file.is_open()) return false;

        writeHeader(file);

        writeMetadata(file, project);

        writeTheme(file, project);

        writeWidgets(file, project);

        file.close();
        return true;
    }

    bool load(const std::string& path, UIProject& project) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        project.clear();

        std::string line;
        std::unordered_map<uint64_t, Widget*> widgetMap;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "MIRUI") {

            } else if (token == "NAME") {
                std::string name;
                std::getline(iss, name);

                size_t start = name.find_first_not_of(" \t");
                size_t end = name.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    name = name.substr(start, end - start + 1);
                }
                project.setName(name);
            } else if (token == "THEME") {

                std::string property;
                std::string value;
                iss >> property >> value;
                if (!property.empty() && !value.empty()) {
                    applyThemeProperty(project, property, value);
                }
            } else if (token == "WIDGET") {

                uint64_t id;
                std::string typeStr, name;
                double x, y, w, h;
                if (iss >> id >> typeStr) {

                    std::string rest;
                    std::getline(iss, rest);

                    size_t q1 = rest.find('"');
                    size_t q2 = rest.find('"', q1 + 1);
                    if (q1 != std::string::npos && q2 != std::string::npos) {
                        name = rest.substr(q1 + 1, q2 - q1 - 1);

                        std::istringstream coordStream(rest.substr(q2 + 1));
                        coordStream >> x >> y >> w >> h;
                    } else {

                        std::istringstream restStream(rest);
                        restStream >> name >> x >> y >> w >> h;
                    }

                    WidgetType type = stringToWidgetType(typeStr);
                    auto widget = WidgetFactory::create(type);
                    if (widget) {
                        widget->setName(name);
                        widget->setBounds(Rect{x, y, w, h});
                        Widget* ptr = widget.get();
                        widgetMap[id] = ptr;

                        if (project.widgetTree().root() == nullptr) {
                            project.widgetTree().setRoot(std::move(widget));
                        } else {

                            project.widgetTree().root()->addChild(widget.release());
                        }
                        project.widgetTree().registerWidget(ptr);
                    }
                }
            } else if (token == "PROPERTY") {

                uint64_t widgetId;
                std::string propName, propValue;
                if (iss >> widgetId >> std::quoted(propName)) {

                    char nextChar;
                    iss >> std::ws;
                    nextChar = iss.peek();
                    if (nextChar == '"') {
                        iss >> std::quoted(propValue);
                    } else {
                        iss >> propValue;
                    }

                    auto it = widgetMap.find(widgetId);
                    if (it != widgetMap.end()) {
                        setPropertyFromString(it->second, propName, propValue);
                    }
                }
            } else if (token == "CHILD") {

                uint64_t parentId, childId;
                if (iss >> parentId >> childId) {
                    auto parentIt = widgetMap.find(parentId);
                    auto childIt = widgetMap.find(childId);
                    if (parentIt != widgetMap.end() && childIt != widgetMap.end()) {
                        Widget* parent = parentIt->second;
                        Widget* child = childIt->second;

                        if (child->parent()) {
                            child->parent()->removeChild(child->id());
                        }
                        parent->addChild(child);
                    }
                }
            }
        }

        file.close();

        LayoutEngine engine;
        engine.layout(project.widgetTree());

        return true;
    }

private:

    void writeHeader(std::ofstream& file) {
        file << "MIRUI 1.0.0\n";
    }

    void writeMetadata(std::ofstream& file, const UIProject& project) {
        file << "# Metadata\n";
        file << "NAME " << project.name() << "\n";
        file << "# End Metadata\n\n";
    }

    void writeTheme(std::ofstream& file, UIProject& project) {
        file << "# Theme\n";
        const Theme& theme = project.themeManager().theme();

        writeThemeColor(file, "colors.background", theme.colors.background);
        writeThemeColor(file, "colors.surface", theme.colors.surface);
        writeThemeColor(file, "colors.surfaceHover", theme.colors.surfaceHover);
        writeThemeColor(file, "colors.surfaceActive", theme.colors.surfaceActive);
        writeThemeColor(file, "colors.textPrimary", theme.colors.textPrimary);
        writeThemeColor(file, "colors.textSecondary", theme.colors.textSecondary);
        writeThemeColor(file, "colors.textMuted", theme.colors.textMuted);
        writeThemeColor(file, "colors.accent", theme.colors.accent);
        writeThemeColor(file, "colors.accentHover", theme.colors.accentHover);
        writeThemeColor(file, "colors.accentActive", theme.colors.accentActive);
        writeThemeColor(file, "colors.border", theme.colors.border);
        writeThemeColor(file, "colors.error", theme.colors.error);
        writeThemeColor(file, "colors.warning", theme.colors.warning);
        writeThemeColor(file, "colors.success", theme.colors.success);

        const Metrics& m = theme.metrics;
        writeThemeDouble(file, "metrics.spacingXS", m.spacingXS);
        writeThemeDouble(file, "metrics.spacingS", m.spacingS);
        writeThemeDouble(file, "metrics.spacingM", m.spacingM);
        writeThemeDouble(file, "metrics.spacingL", m.spacingL);
        writeThemeDouble(file, "metrics.spacingXL", m.spacingXL);
        writeThemeDouble(file, "metrics.radiusS", m.radiusS);
        writeThemeDouble(file, "metrics.radiusM", m.radiusM);
        writeThemeDouble(file, "metrics.radiusL", m.radiusL);
        writeThemeDouble(file, "metrics.borderWidth", m.borderWidth);
        writeThemeDouble(file, "metrics.controlHeight", m.controlHeight);
        writeThemeDouble(file, "metrics.toolbarHeight", m.toolbarHeight);
        writeThemeDouble(file, "metrics.panelWidth", m.panelWidth);

        file << "# End Theme\n\n";
    }

    void writeWidgets(std::ofstream& file, UIProject& project) {
        file << "# Widgets\n";

        project.widgetTree().forEach([&](Widget* widget) {

            file << "WIDGET " << widget->id().value()
                 << " " << widgetTypeToString(widget->type())
                 << " \"" << widget->name() << "\""
                 << " " << widget->bounds().x
                 << " " << widget->bounds().y
                 << " " << widget->bounds().width
                 << " " << widget->bounds().height << "\n";

            for (const auto& [key, value] : widget->allProperties()) {

                if (key == "name" || key == "type" || key == "x" || key == "y" ||
                    key == "width" || key == "height") continue;

                file << "PROPERTY " << widget->id().value()
                     << " \"" << key << "\" ";

                std::visit([&file](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        file << (v ? "true" : "false");
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        file << v;
                    } else if constexpr (std::is_same_v<T, double>) {
                        file << v;
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        file << "\"" << v << "\"";
                    } else {
                        file << "\"\"";
                    }
                }, value);
                file << "\n";
            }

            if (widget->parent()) {
                file << "CHILD " << widget->parent()->id().value()
                     << " " << widget->id().value() << "\n";
            }
        });
        file << "# End Widgets\n\n";
    }

    void writeThemeColor(std::ofstream& file, const std::string& name, const Color& color) {
        file << "THEME " << name << " #"
             << std::hex << static_cast<int>(color.r * 255)
             << static_cast<int>(color.g * 255)
             << static_cast<int>(color.b * 255)
             << static_cast<int>(color.a * 255) << std::dec << "\n";
    }

    void writeThemeDouble(std::ofstream& file, const std::string& name, double value) {
        file << "THEME " << name << " " << value << "\n";
    }

    void applyThemeProperty(UIProject& project, const std::string& property, const std::string& value) {
        Theme& theme = project.themeManager().theme();

        if (value[0] == '#') {

            if (value.length() == 9) {
                uint32_t hex = std::stoul(value.substr(1), nullptr, 16);
                Color color;
                color.r = ((hex >> 24) & 0xFF) / 255.0f;
                color.g = ((hex >> 16) & 0xFF) / 255.0f;
                color.b = ((hex >> 8) & 0xFF) / 255.0f;
                color.a = (hex & 0xFF) / 255.0f;
                setThemeColor(theme.colors, property, color);
            }
        } else {

            double val = std::stod(value);
            setThemeMetric(theme.metrics, property, val);
        }
    }

    void setThemeColor(ColorPalette& palette, const std::string& name, const Color& color) {
        if (name == "colors.background") palette.background = color;
        else if (name == "colors.surface") palette.surface = color;
        else if (name == "colors.surfaceHover") palette.surfaceHover = color;
        else if (name == "colors.surfaceActive") palette.surfaceActive = color;
        else if (name == "colors.textPrimary") palette.textPrimary = color;
        else if (name == "colors.textSecondary") palette.textSecondary = color;
        else if (name == "colors.textMuted") palette.textMuted = color;
        else if (name == "colors.accent") palette.accent = color;
        else if (name == "colors.accentHover") palette.accentHover = color;
        else if (name == "colors.accentActive") palette.accentActive = color;
        else if (name == "colors.border") palette.border = color;
        else if (name == "colors.error") palette.error = color;
        else if (name == "colors.warning") palette.warning = color;
        else if (name == "colors.success") palette.success = color;
    }

    void setThemeMetric(Metrics& metrics, const std::string& name, double value) {
        if (name == "metrics.spacingXS") metrics.spacingXS = value;
        else if (name == "metrics.spacingS") metrics.spacingS = value;
        else if (name == "metrics.spacingM") metrics.spacingM = value;
        else if (name == "metrics.spacingL") metrics.spacingL = value;
        else if (name == "metrics.spacingXL") metrics.spacingXL = value;
        else if (name == "metrics.radiusS") metrics.radiusS = value;
        else if (name == "metrics.radiusM") metrics.radiusM = value;
        else if (name == "metrics.radiusL") metrics.radiusL = value;
        else if (name == "metrics.borderWidth") metrics.borderWidth = value;
        else if (name == "metrics.controlHeight") metrics.controlHeight = value;
        else if (name == "metrics.toolbarHeight") metrics.toolbarHeight = value;
        else if (name == "metrics.panelWidth") metrics.panelWidth = value;
    }

    static WidgetType stringToWidgetType(const std::string& str) {
        if (str == "Window")       return WidgetType::Window;
        if (str == "Panel")        return WidgetType::Panel;
        if (str == "Button")       return WidgetType::Button;
        if (str == "Label")        return WidgetType::Label;
        if (str == "Tree")         return WidgetType::Tree;
        if (str == "PropertyGrid") return WidgetType::PropertyGrid;
        if (str == "Toolbar")      return WidgetType::Toolbar;
        if (str == "Ribbon")       return WidgetType::Ribbon;
        if (str == "DockPanel")    return WidgetType::DockPanel;
        if (str == "Viewport")     return WidgetType::Viewport;
        if (str == "Timeline")     return WidgetType::Timeline;
        return WidgetType::Unknown;
    }

    static std::string widgetTypeToString(WidgetType type) {
        switch (type) {
            case WidgetType::Window:       return "Window";
            case WidgetType::Panel:        return "Panel";
            case WidgetType::Button:       return "Button";
            case WidgetType::Label:        return "Label";
            case WidgetType::Tree:         return "Tree";
            case WidgetType::PropertyGrid: return "PropertyGrid";
            case WidgetType::Toolbar:      return "Toolbar";
            case WidgetType::Ribbon:       return "Ribbon";
            case WidgetType::DockPanel:    return "DockPanel";
            case WidgetType::Viewport:     return "Viewport";
            case WidgetType::Timeline:     return "Timeline";
            default:                       return "Unknown";
        }
    }

    static void setPropertyFromString(Widget* widget, const std::string& name, const std::string& value) {
        if (value == "true") {
            widget->setProperty(name, StateValue(true));
        } else if (value == "false") {
            widget->setProperty(name, StateValue(false));
        } else {

            try {
                size_t pos;
                double d = std::stod(value, &pos);
                if (pos == value.size()) {

                    if (d == static_cast<int64_t>(d) && d >= static_cast<double>(INT64_MIN) && d <= static_cast<double>(INT64_MAX)) {
                        widget->setProperty(name, StateValue(static_cast<int64_t>(d)));
                    } else {
                        widget->setProperty(name, StateValue(d));
                    }
                } else {

                    widget->setProperty(name, StateValue(value));
                }
            } catch (...) {

                widget->setProperty(name, StateValue(value));
            }
        }
    }
};

}