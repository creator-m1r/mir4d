
#pragma once

#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Widget/Widget.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace MirUI {

enum class GuideType {
    LeftEdge,
    RightEdge,
    TopEdge,
    BottomEdge,
    HorizontalCenter,
    VerticalCenter
};

struct Guide {
    GuideType type;
    double    position;
};

class GuideManager {
public:
    GuideManager()
        : m_snapThreshold(4.0)
        , m_enabled(true)
    {}

    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    void setSnapThreshold(double threshold) { m_snapThreshold = threshold; }
    [[nodiscard]] double snapThreshold() const { return m_snapThreshold; }

    void generateGuides(const std::vector<Widget*>& widgets) {
        m_guides.clear();
        for (const Widget* w : widgets) {
            if (!w || !w->isVisible()) continue;
            Rect bounds = w->bounds();

            m_guides.push_back({GuideType::LeftEdge, bounds.x});
            m_guides.push_back({GuideType::RightEdge, bounds.x + bounds.width});
            m_guides.push_back({GuideType::VerticalCenter, bounds.x + bounds.width * 0.5});

            m_guides.push_back({GuideType::TopEdge, bounds.y});
            m_guides.push_back({GuideType::BottomEdge, bounds.y + bounds.height});
            m_guides.push_back({GuideType::HorizontalCenter, bounds.y + bounds.height * 0.5});
        }

        std::sort(m_guides.begin(), m_guides.end(),
            [](const Guide& a, const Guide& b) {
                if (a.type != b.type) return a.type < b.type;
                return a.position < b.position;
            });
        auto last = std::unique(m_guides.begin(), m_guides.end(),
            [](const Guide& a, const Guide& b) {
                return a.type == b.type && std::abs(a.position - b.position) < 0.1;
            });
        m_guides.erase(last, m_guides.end());
    }

    [[nodiscard]] Rect snap(const Rect& rect) {
        m_activeGuides.clear();
        if (!m_enabled) return rect;

        Rect result = rect;

        snapEdge(result.x, GuideType::LeftEdge,
            [&](double delta) { result.x += delta; });
        snapEdge(result.x + result.width, GuideType::RightEdge,
            [&](double delta) { result.x += delta; });
        snapEdge(result.x + result.width * 0.5, GuideType::VerticalCenter,
            [&](double delta) { result.x += delta; });

        snapEdge(result.y, GuideType::TopEdge,
            [&](double delta) { result.y += delta; });
        snapEdge(result.y + result.height, GuideType::BottomEdge,
            [&](double delta) { result.y += delta; });
        snapEdge(result.y + result.height * 0.5, GuideType::HorizontalCenter,
            [&](double delta) { result.y += delta; });

        return result;
    }

    [[nodiscard]] const std::vector<Guide>& activeGuides() const { return m_activeGuides; }

    void clear() {
        m_guides.clear();
        m_activeGuides.clear();
    }

private:
    double m_snapThreshold;
    bool   m_enabled;

    std::vector<Guide> m_guides;
    std::vector<Guide> m_activeGuides;

    void snapEdge(double edgeValue, GuideType targetType,
                  const std::function<void(double)>& applyDelta) {
        double bestDist = m_snapThreshold + 1.0;
        double bestPos = edgeValue;

        for (const Guide& guide : m_guides) {
            if (guide.type != targetType) continue;
            double dist = std::abs(edgeValue - guide.position);
            if (dist < bestDist) {
                bestDist = dist;
                bestPos = guide.position;
            }
        }

        if (bestDist <= m_snapThreshold) {
            applyDelta(bestPos - edgeValue);
            m_activeGuides.push_back({targetType, bestPos});
        }
    }
};

}