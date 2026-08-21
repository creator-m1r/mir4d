
#import <Foundation/Foundation.h>
#include "SwiftUIRenderer.hpp"
#include "MirUI-CAPI.h"
#include "../../Core/UIContext.hpp"
#include "../../Core/Theme/ThemeManager.hpp"
#include "../../Core/Theme/ThemeID.hpp"
#include "../../Core/Theme/ThemeResolver.hpp"
#include "../../Core/Theme/WidgetStyle.hpp"
#include "../../Designer/Canvas/AlignmentManager.hpp"
#include "../../Foundation/Animation/AnimationSpec.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sstream>

static std::unique_ptr<MirUI::UIContext>        g_context;
static std::unique_ptr<MirUI::SwiftUIRenderer>  g_renderer;

static MirUI::WidgetType stringToWidgetType(const std::string& str) {
    if (str == "Button")        return MirUI::WidgetType::Button;
    if (str == "Label")         return MirUI::WidgetType::Label;
    if (str == "Panel")         return MirUI::WidgetType::Panel;
    if (str == "Toolbar")       return MirUI::WidgetType::Toolbar;
    if (str == "CheckBox")      return MirUI::WidgetType::CheckBox;
    if (str == "TextField")     return MirUI::WidgetType::TextField;
    if (str == "ComboBox")      return MirUI::WidgetType::ComboBox;
    if (str == "Slider")        return MirUI::WidgetType::Slider;
    if (str == "RadioButton")   return MirUI::WidgetType::RadioButton;
    if (str == "ProgressBar")   return MirUI::WidgetType::ProgressBar;
    if (str == "Image")         return MirUI::WidgetType::Image;
    if (str == "TableView")     return MirUI::WidgetType::TableView;
    if (str == "ScrollView")    return MirUI::WidgetType::ScrollView;
    if (str == "TabView")       return MirUI::WidgetType::TabView;
    if (str == "Viewport")      return MirUI::WidgetType::Viewport;
    if (str == "Window")        return MirUI::WidgetType::Window;
    return MirUI::WidgetType::Unknown;
}

static MirUI::WidgetState stringToWidgetState(const std::string& str) {
    if (str == "Normal")    return MirUI::WidgetState::Normal;
    if (str == "Hover")     return MirUI::WidgetState::Hover;
    if (str == "Pressed")   return MirUI::WidgetState::Pressed;
    if (str == "Disabled")  return MirUI::WidgetState::Disabled;
    if (str == "Focused")   return MirUI::WidgetState::Focused;
    if (str == "Selected")  return MirUI::WidgetState::Selected;
    return MirUI::WidgetState::Normal;
}

extern "C" void MirUI_Init() {
    g_context = std::make_unique<MirUI::UIContext>();
    g_renderer = std::make_unique<MirUI::SwiftUIRenderer>();
    g_renderer->setUIContext(g_context.get());

    g_context->themeManager().registerTheme(MirUI::Theme::createLight());
    g_context->themeManager().registerTheme(MirUI::Theme::createDark());

    NSLog(@"[MirUICppBridge] Инициализация завершена (UIContext).");
}

extern "C" void MirUI_Shutdown() {
    g_renderer.reset();
    g_context.reset();
}

extern "C" int64_t MirUI_AddWidget(const char* widgetType, double x, double y, double w, double h) {
    if (!g_context) return 0;
    
    std::string typeStr(widgetType);
    MirUI::WidgetType type = stringToWidgetType(typeStr);
    if (type == MirUI::WidgetType::Unknown) {
        NSLog(@"[MirUICppBridge] Неизвестный тип виджета: '%s'", widgetType);
        return 0;
    }

    MirUI::WidgetID rootId;
    if (g_context->widgetTree().root()) {
        rootId = g_context->widgetTree().root()->id();
    }
    MirUI::WidgetID widgetId = g_context->addWidget(type, rootId);
    if (widgetId.value() == 0) return 0;

    MirUI::Widget* widget = g_context->widgetTree().find(widgetId);
    if (widget) {

        MirUI::Rect bounds = widget->bounds();
        bounds.x = x; bounds.y = y; bounds.width = w; bounds.height = h;
        widget->setBounds(bounds);

        switch (type) {
            case MirUI::WidgetType::Button:
                widget->setProperty("text", MirUI::StateValue(std::string("Кнопка")));
                break;
            case MirUI::WidgetType::Label:
                widget->setProperty("text", MirUI::StateValue(std::string("Надпись")));
                break;
            case MirUI::WidgetType::CheckBox:
                widget->setProperty("text", MirUI::StateValue(std::string("Флажок")));
                widget->setProperty("checked", MirUI::StateValue(false));
                break;
            case MirUI::WidgetType::TextField:
                widget->setProperty("placeholder", MirUI::StateValue(std::string("Введите текст...")));
                break;
            case MirUI::WidgetType::ComboBox:
                widget->setProperty("items", MirUI::StateValue(std::string("Вариант 1|Вариант 2|Вариант 3")));
                widget->setProperty("selectedIndex", MirUI::StateValue(static_cast<int64_t>(0)));
                break;
            case MirUI::WidgetType::Slider:
                widget->setProperty("value", MirUI::StateValue(50.0));
                widget->setProperty("minValue", MirUI::StateValue(0.0));
                widget->setProperty("maxValue", MirUI::StateValue(100.0));
                break;
            case MirUI::WidgetType::RadioButton:
                widget->setProperty("text", MirUI::StateValue(std::string("Переключатель")));
                widget->setProperty("selected", MirUI::StateValue(false));
                break;
            case MirUI::WidgetType::ProgressBar:
                widget->setProperty("value", MirUI::StateValue(0.0));
                widget->setProperty("minValue", MirUI::StateValue(0.0));
                widget->setProperty("maxValue", MirUI::StateValue(100.0));
                break;
            default: break;
        }
    }

    MirUI_RenderFrame();
    NSLog(@"[MirUICppBridge] Добавлен виджет типа '%s' с ID=%llu", widgetType, widgetId.value());
    return static_cast<int64_t>(widgetId.value());
}

extern "C" void MirUI_AddButton(const char* text, double x, double y, double w, double h) {
    int64_t id = MirUI_AddWidget("Button", x, y, w, h);
    if (id != 0 && g_context) {
        MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(id)));
        if (widget) {
            widget->setProperty("text", MirUI::StateValue(std::string(text)));
        }
        MirUI_RenderFrame();
    }
}

extern "C" void MirUI_MoveWidget(int64_t widgetId, double dx, double dy) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    MirUI::Rect bounds = widget->bounds();
    bounds.x += dx; bounds.y += dy;
    widget->setBounds(bounds);
    MirUI_RenderFrame();
}

extern "C" void MirUI_ResizeWidget(int64_t widgetId, double newWidth, double newHeight, double newX, double newY) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    widget->setBounds(MirUI::Rect(newX, newY, newWidth, newHeight));
    MirUI_RenderFrame();
}

extern "C" void MirUI_DeleteWidget(int64_t widgetId) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    MirUI::Widget* parent = widget->parent();
    if (parent) {
        parent->removeChild(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
        g_context->widgetTree().unregisterWidget(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
        delete widget;
    }
    MirUI_RenderFrame();
}

extern "C" void MirUI_SetPropertyString(int64_t widgetId, const char* propertyName, const char* value) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    widget->setProperty(std::string(propertyName), MirUI::StateValue(std::string(value)));
    MirUI_RenderFrame();
}

extern "C" void MirUI_SetPropertyDouble(int64_t widgetId, const char* propertyName, double value) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    widget->setProperty(std::string(propertyName), MirUI::StateValue(value));
    MirUI_RenderFrame();
}

extern "C" void MirUI_SetPropertyBool(int64_t widgetId, const char* propertyName, bool value) {
    if (!g_context) return;
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    widget->setProperty(std::string(propertyName), MirUI::StateValue(value));
    MirUI_RenderFrame();
}

extern "C" const char* MirUI_GetPropertyString(int64_t widgetId, const char* propertyName) {
    if (!g_context) return strdup("");
    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return strdup("");
    auto val = widget->getProperty(std::string(propertyName));
    if (!val.has_value()) return strdup("");
    if (std::holds_alternative<std::string>(*val)) return strdup(std::get<std::string>(*val).c_str());
    if (std::holds_alternative<double>(*val)) return strdup(std::to_string(std::get<double>(*val)).c_str());
    if (std::holds_alternative<int64_t>(*val)) return strdup(std::to_string(std::get<int64_t>(*val)).c_str());
    if (std::holds_alternative<bool>(*val)) return strdup(std::get<bool>(*val) ? "true" : "false");
    return strdup("");
}

extern "C" void MirUI_SwitchTheme(const char* themeId) {
    if (!g_context) return;
    g_context->switchTheme(MirUI::ThemeID(std::string(themeId)));
    MirUI_RenderFrame();
}

extern "C" const char* MirUI_CurrentThemeName() {
    if (!g_context) return strdup("Светлая тема");
    return strdup(g_context->themeManager().current().name.c_str());
}

extern "C" void MirUI_RegisterTheme(const char* themeId) {
    if (!g_context) return;
    std::string id(themeId);
    if (id == "mir.light") {
        g_context->themeManager().registerTheme(MirUI::Theme::createLight());
    } else if (id == "mir.dark") {
        g_context->themeManager().registerTheme(MirUI::Theme::createDark());
    }
    NSLog(@"[MirUICppBridge] Тема '%s' зарегистрирована.", themeId);
}

extern "C" const char* MirUI_GetThemeColor(const char* colorToken) {
    if (!g_context) return strdup("#000000FF");
    MirUI::Theme theme = g_context->themeManager().current();
    std::string token(colorToken);
    MirUI::Color color;
    if (token == "interface.background") color = theme.colors.background;
    else if (token == "interface.surface") color = theme.colors.surface;
    else if (token == "interface.textPrimary") color = theme.colors.textPrimary;
    else if (token == "interface.accent") color = theme.colors.accent;
    else if (token == "interface.border") color = theme.colors.border;
    else color = MirUI::Color::black();
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X",
             (int)(color.r * 255), (int)(color.g * 255),
             (int)(color.b * 255), (int)(color.a * 255));
    return strdup(hex);
}

extern "C" void MirUI_SetThemeColor(const char* colorToken, const char* hexColor) {
    if (!g_context) return;
    std::string token(colorToken);
    std::string hex(hexColor);
    if (hex.length() != 9 || hex[0] != '#') return;
    unsigned int r, g, b, a;
    sscanf(hex.c_str() + 1, "%02X%02X%02X%02X", &r, &g, &b, &a);
    MirUI::Color color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    MirUI::Theme theme = g_context->themeManager().current();
    if (token == "interface.background") theme.colors.background = color;
    else if (token == "interface.surface") theme.colors.surface = color;
    else if (token == "interface.textPrimary") theme.colors.textPrimary = color;
    else if (token == "interface.accent") theme.colors.accent = color;
    else if (token == "interface.border") theme.colors.border = color;
    g_context->themeManager().setTheme(theme);
    MirUI_RenderFrame();
}

extern "C" double MirUI_GetThemeMetric(const char* metricToken) {
    if (!g_context) return 0.0;
    MirUI::Theme theme = g_context->themeManager().current();
    std::string token(metricToken);
    if (token == "spacing.m") return theme.metrics.spacingM;
    if (token == "toolbar.height") return theme.metrics.toolbarHeight;
    if (token == "radius.m") return theme.metrics.radiusM;

    return 0.0;
}

extern "C" void MirUI_SetThemeMetric(const char* metricToken, double value) {
    if (!g_context) return;
    std::string token(metricToken);
    MirUI::Theme theme = g_context->themeManager().current();
    if (token == "spacing.m") theme.metrics.spacingM = value;
    else if (token == "toolbar.height") theme.metrics.toolbarHeight = value;
    else if (token == "radius.m") theme.metrics.radiusM = value;
    g_context->themeManager().setTheme(theme);
    MirUI_RenderFrame();
}

extern "C" const char* MirUI_GetThemeFont(const char* fontToken) {
    if (!g_context) return strdup("System;14;400;0");
    MirUI::Theme theme = g_context->themeManager().current();
    std::string token(fontToken);
    MirUI::Font font;
    if (token == "typography.body") font = theme.typography.body;
    else if (token == "typography.button") font = theme.typography.button;
    else font = theme.typography.body;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s;%f;%d;%d",
             font.family.c_str(), font.size,
             static_cast<int>(font.weight), static_cast<int>(font.style));
    return strdup(buf);
}

extern "C" void MirUI_SetThemeFont(const char* fontToken, const char* fontString) {
    if (!g_context) return;
    std::string token(fontToken);
    std::string fontStr(fontString);
    std::vector<std::string> parts;
    size_t start = 0, end;
    while ((end = fontStr.find(';', start)) != std::string::npos) {
        parts.push_back(fontStr.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(fontStr.substr(start));
    if (parts.size() < 4) return;
    MirUI::Font font;
    font.family = parts[0];
    font.size = std::stod(parts[1]);
    font.weight = static_cast<MirUI::FontWeight>(std::stoi(parts[2]));
    font.style = static_cast<MirUI::FontStyle>(std::stoi(parts[3]));
    MirUI::Theme theme = g_context->themeManager().current();
    if (token == "typography.body") theme.typography.body = font;
    else if (token == "typography.button") theme.typography.button = font;
    g_context->themeManager().setTheme(theme);
    MirUI_RenderFrame();
}

extern "C" const char* MirUI_GetThemeShadow(const char* shadowToken) {
    if (!g_context) return strdup("0,0,0,0.15,0,2,4");

    return strdup("0,0,0,0.15,0,2,4");
}

extern "C" void MirUI_SetThemeShadow(const char* shadowToken, const char* shadowString) {
    if (!g_context) return;

    MirUI_RenderFrame();
}

extern "C" double MirUI_GetThemeAnimationDuration() {
    if (!g_context) return 0.25;
    return g_context->themeManager().current().animations.defaultDuration;
}

extern "C" void MirUI_SetThemeAnimationDuration(double duration) {
    if (!g_context) return;
    MirUI::Theme theme = g_context->themeManager().current();
    theme.animations.defaultDuration = duration;
    g_context->themeManager().setTheme(theme);
    MirUI_RenderFrame();
}

extern "C" void MirUI_CopyWidget(int64_t widgetId) {
    if (!g_context) return;

    MirUI::Widget* source = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!source) return;

}

extern "C" void MirUI_PasteWidget(int64_t parentId) {
    if (!g_context) return;
    MirUI::WidgetID pid = (parentId == 0) ? MirUI::WidgetID{} : MirUI::WidgetID(static_cast<uint64_t>(parentId));

}

extern "C" void MirUI_CutWidget(int64_t widgetId) {
    if (!g_context) return;
    MirUI_CopyWidget(widgetId);
    MirUI_DeleteWidget(widgetId);
    MirUI_RenderFrame();
}

extern "C" void MirUI_AlignWidgets(const int64_t* widgetIds, int count, const char* strategy) {
    if (!g_context || count <= 1) return;
    std::string strat(strategy);
    MirUI::AlignStrategy align = MirUI::AlignStrategy::Left;
    if (strat == "Left") align = MirUI::AlignStrategy::Left;
    else if (strat == "CenterHorizontal") align = MirUI::AlignStrategy::CenterHorizontal;
    else if (strat == "Right") align = MirUI::AlignStrategy::Right;
    else if (strat == "Top") align = MirUI::AlignStrategy::Top;
    else if (strat == "CenterVertical") align = MirUI::AlignStrategy::CenterVertical;
    else if (strat == "Bottom") align = MirUI::AlignStrategy::Bottom;
    else if (strat == "DistributeHorizontal") align = MirUI::AlignStrategy::DistributeHorizontal;
    else if (strat == "DistributeVertical") align = MirUI::AlignStrategy::DistributeVertical;

    std::vector<MirUI::Widget*> widgets;
    for (int i = 0; i < count; ++i) {
        MirUI::Widget* w = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetIds[i])));
        if (w) widgets.push_back(w);
    }
    if (widgets.size() < 2) return;

    MirUI::AlignmentManager aligner;
    aligner.align(widgets, align);
    MirUI_RenderFrame();
}

extern "C" void MirUI_NewProject() {
    if (!g_context) return;

    g_context->widgetTree().clear();
    g_context->clearHistory();
    MirUI_RenderFrame();
}

extern "C" bool MirUI_SaveProject(const char* path) {
    if (!g_context) return false;

    return g_context->saveProject(std::string(path));
}

extern "C" bool MirUI_LoadProject(const char* path) {
    if (!g_context) return false;
    bool result = g_context->loadProject(std::string(path));
    if (result) MirUI_RenderFrame();
    return result;
}

extern "C" void MirUI_EnterPreview() {
    if (!g_context) return;
    g_context->enterPreview();
}

extern "C" void MirUI_ExitPreview() {
    if (!g_context) return;
    g_context->exitPreview();
    MirUI_RenderFrame();
}

extern "C" void MirUI_TogglePreview() {
    if (!g_context) return;
    g_context->togglePreview();
    MirUI_RenderFrame();
}

extern "C" void MirUI_Undo() {
    if (!g_context) return;
    g_context->undo();
    MirUI_RenderFrame();
}

extern "C" void MirUI_Redo() {
    if (!g_context) return;
    g_context->redo();
    MirUI_RenderFrame();
}

extern "C" void MirUI_AnimateProperty(int64_t widgetId,
                                      const char* propertyName,
                                      const char* targetValue,
                                      double duration,
                                      const char* curveType) {
    if (!g_context) return;
    std::string valStr(targetValue);
    MirUI::StateValue value;
    if (valStr == "true") value = MirUI::StateValue(true);
    else if (valStr == "false") value = MirUI::StateValue(false);
    else {
        try {
            size_t pos;
            double d = std::stod(valStr, &pos);
            if (pos == valStr.size()) {
                if (d == static_cast<int64_t>(d) && d >= static_cast<double>(INT64_MIN) && d <= static_cast<double>(INT64_MAX))
                    value = MirUI::StateValue(static_cast<int64_t>(d));
                else
                    value = MirUI::StateValue(d);
            } else {
                value = MirUI::StateValue(valStr);
            }
        } catch (...) {
            value = MirUI::StateValue(valStr);
        }
    }

    std::string curveStr(curveType);
    MirUI::AnimationSpec spec;
    spec.duration = duration;
    if (curveStr == "linear") spec.curve = MirUI::AnimationCurve::Linear;
    else if (curveStr == "easeIn") spec.curve = MirUI::AnimationCurve::EaseIn;
    else if (curveStr == "easeOut") spec.curve = MirUI::AnimationCurve::EaseOut;
    else if (curveStr == "easeInOut") spec.curve = MirUI::AnimationCurve::EaseInOut;
    else if (curveStr == "spring") {
        spec.curve = MirUI::AnimationCurve::Spring;
        spec.springDamping = 0.6;
        spec.springResponse = 0.4;
    }

    MirUI::Widget* widget = g_context->widgetTree().find(MirUI::WidgetID(static_cast<uint64_t>(widgetId)));
    if (!widget) return;
    g_context->animationManager().animate(*widget, std::string(propertyName), value, spec);
}

extern "C" const char* MirUI_GetWidgetStyleField(const char* widgetTypeStr,
                                                  const char* widgetStateStr,
                                                  const char* fieldName) {
    if (!g_context) return strdup("");

    MirUI::WidgetType type = stringToWidgetType(std::string(widgetTypeStr));
    MirUI::WidgetState state = stringToWidgetState(std::string(widgetStateStr));

    MirUI::Theme theme = g_context->themeManager().current();
    MirUI::ThemeResolver resolver(theme);
    MirUI::WidgetStyle style = resolver.resolve(type, state);

    std::string field(fieldName);
    std::string result;

    if (field == "background") {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X",
                 (int)(style.background.r * 255), (int)(style.background.g * 255),
                 (int)(style.background.b * 255), (int)(style.background.a * 255));
        result = hex;
    } else if (field == "foreground") {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X",
                 (int)(style.foreground.r * 255), (int)(style.foreground.g * 255),
                 (int)(style.foreground.b * 255), (int)(style.foreground.a * 255));
        result = hex;
    } else if (field == "border") {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X",
                 (int)(style.border.r * 255), (int)(style.border.g * 255),
                 (int)(style.border.b * 255), (int)(style.border.a * 255));
        result = hex;
    } else if (field == "cornerRadius") {
        result = std::to_string(style.cornerRadius);
    } else if (field == "opacity") {
        result = std::to_string(style.opacity);
    } else if (field == "fontFamily") {
        result = style.font.family;
    } else if (field == "fontSize") {
        result = std::to_string(style.font.size);
    } else if (field == "fontWeight") {
        result = std::to_string(static_cast<int>(style.font.weight));
    } else if (field == "fontStyle") {
        result = std::to_string(static_cast<int>(style.font.style));
    } else if (field == "shadowColor") {
        char hex[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X",
                 (int)(style.shadow.color.r * 255), (int)(style.shadow.color.g * 255),
                 (int)(style.shadow.color.b * 255), (int)(style.shadow.color.a * 255));
        result = hex;
    } else if (field == "shadowOffsetX") {
        result = std::to_string(style.shadow.offsetX);
    } else if (field == "shadowOffsetY") {
        result = std::to_string(style.shadow.offsetY);
    } else if (field == "shadowBlur") {
        result = std::to_string(style.shadow.blurRadius);
    } else if (field == "visible") {
        result = style.visible ? "true" : "false";
    }
    return strdup(result.c_str());
}

extern "C" void MirUI_SetWidgetStyleField(const char* widgetTypeStr,
                                          const char* widgetStateStr,
                                          const char* fieldName,
                                          const char* value) {
    if (!g_context) return;

    MirUI::WidgetType type = stringToWidgetType(std::string(widgetTypeStr));
    MirUI::WidgetState state = stringToWidgetState(std::string(widgetStateStr));

    MirUI::Theme theme = g_context->themeManager().current();
    MirUI::ThemeResolver resolver(theme);
    MirUI::WidgetStyle style = resolver.resolve(type, state);

    std::string field(fieldName);
    std::string val(value);

    if (field == "background" || field == "foreground" || field == "border" || field == "shadowColor") {
        if (val.length() == 9 && val[0] == '#') {
            unsigned int r, g, b, a;
            sscanf(val.c_str() + 1, "%02X%02X%02X%02X", &r, &g, &b, &a);
            MirUI::Color color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
            if (field == "background") style.background = color;
            else if (field == "foreground") style.foreground = color;
            else if (field == "border") style.border = color;
            else if (field == "shadowColor") style.shadow.color = color;
        }
    } else if (field == "cornerRadius" || field == "opacity" ||
               field == "fontSize" || field == "fontWeight" || field == "fontStyle" ||
               field == "shadowOffsetX" || field == "shadowOffsetY" || field == "shadowBlur") {
        double d = std::stod(val);
        if (field == "cornerRadius") style.cornerRadius = d;
        else if (field == "opacity") style.opacity = d;
        else if (field == "fontSize") style.font.size = d;
        else if (field == "fontWeight") style.font.weight = static_cast<MirUI::FontWeight>(static_cast<int>(d));
        else if (field == "fontStyle") style.font.style = static_cast<MirUI::FontStyle>(static_cast<int>(d));
        else if (field == "shadowOffsetX") style.shadow.offsetX = d;
        else if (field == "shadowOffsetY") style.shadow.offsetY = d;
        else if (field == "shadowBlur") style.shadow.blurRadius = d;
    } else if (field == "fontFamily") {
        style.font.family = val;
    } else if (field == "visible") {
        style.visible = (val == "true" || val == "1");
    }

    g_context->themeManager().setWidgetStyle(type, state, style);
    MirUI_RenderFrame();
}

extern "C" void MirUI_RenderFrame() {
    if (!g_context || !g_renderer) return;
    g_context->update(0.016);
    g_renderer->beginFrame();
    g_renderer->render(g_context->widgetTree());
    g_renderer->endFrame();
}