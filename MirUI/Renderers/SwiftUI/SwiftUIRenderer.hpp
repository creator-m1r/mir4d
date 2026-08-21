
#pragma once

#include "../../Core/Bridge/UIBridge.hpp"
#include "../../Core/Widget/WidgetSnapshot.hpp"
#include "../../Core/Theme/ThemeResolver.hpp"
#include "../../Core/Theme/WidgetStyle.hpp"
#include "../../Core/Theme/ThemeManager.hpp"
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace MirUI {

struct SwiftUIViewNode {
    std::string type;
    int64_t widgetId;
    std::string text;
    std::string iconName;
    std::string commandId;
    std::string placeholder;
    std::string items;
    std::string groupId;
    std::string source;
    std::string fit;
    std::string columns;
    std::string data;
    std::string scrollDirection;
    std::string tabTitles;
    std::string tabPosition;
    double x, y, width, height;
    bool visible = true;
    bool checked = false;
    double value = 0.0;
    double minValue = 0.0;
    double maxValue = 100.0;
    bool indeterminate = false;
    int selectedIndex = -1;
    int64_t selectedRow = -1;
    int64_t selectedTab = 0;
    int parentIndex = -1;
    double opacity = 1.0;
    bool showScrollBars = true;

    std::string backgroundHex;
    std::string foregroundHex;
    std::string borderHex;
    double cornerRadius = 0.0;
    std::string fontFamily;
    double fontSize = 14.0;
    int fontWeight = 400;
    int fontStyle = 0;
    double shadowOffsetX = 0.0;
    double shadowOffsetY = 2.0;
    double shadowBlur = 4.0;
    std::string shadowColorHex;
};

class SwiftUIRenderer {
public:
    SwiftUIRenderer();
    ~SwiftUIRenderer();

    void setBridge(UIBridge* bridge) { m_bridge = bridge; }

    void setThemeManager(ThemeManager* manager) { m_themeManager = manager; }

    void render(const WidgetTreeSnapshot& snapshot);

    void requestUpdate(int64_t widgetId);

    [[nodiscard]] const std::vector<SwiftUIViewNode>& viewNodes() const { return m_viewNodes; }

    void clear();

private:
    UIBridge* m_bridge = nullptr;
    ThemeManager* m_themeManager = nullptr;

    std::vector<SwiftUIViewNode> m_viewNodes;
    int m_currentParentIndex = -1;

    void processSnapshot(const WidgetSnapshot& snapshot, int parentIndex);
    SwiftUIViewNode createNode(const WidgetSnapshot& snapshot, int parentIndex);
    void applyStyle(SwiftUIViewNode& node, WidgetType type, const std::string& stateStr);

    static std::string colorToHex(const Color& color);
    static int fontWeightToInt(FontWeight weight);
    static WidgetType stringToWidgetType(const std::string& str);
};

}