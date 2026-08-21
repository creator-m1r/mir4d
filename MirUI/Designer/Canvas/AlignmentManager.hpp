
#pragma once

#include "../../Core/Layout/Rect.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace MirUI {

enum class AlignStrategy {
    Left,
    CenterHorizontal,
    Right,
    Top,
    CenterVertical,
    Bottom,
    DistributeHorizontal,
    DistributeVertical
};

class AlignmentManager {
public:

    [[nodiscard]] static std::vector<Rect> align(const std::vector<Rect>& rects,
                                                  AlignStrategy strategy) {
        if (rects.empty()) return {};

        switch (strategy) {
            case AlignStrategy::Left:
                return alignLeft(rects);
            case AlignStrategy::CenterHorizontal:
                return alignCenterHorizontal(rects);
            case AlignStrategy::Right:
                return alignRight(rects);
            case AlignStrategy::Top:
                return alignTop(rects);
            case AlignStrategy::CenterVertical:
                return alignCenterVertical(rects);
            case AlignStrategy::Bottom:
                return alignBottom(rects);
            case AlignStrategy::DistributeHorizontal:
                return distributeHorizontal(rects);
            case AlignStrategy::DistributeVertical:
                return distributeVertical(rects);
        }
        return rects;
    }

private:

    static std::vector<Rect> alignLeft(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetX = rects[0].x;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            result.push_back(Rect{targetX, r.y, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> alignCenterHorizontal(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetCenter = rects[0].x + rects[0].width * 0.5;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            double newX = targetCenter - r.width * 0.5;
            result.push_back(Rect{newX, r.y, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> alignRight(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetRight = rects[0].x + rects[0].width;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            double newX = targetRight - r.width;
            result.push_back(Rect{newX, r.y, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> alignTop(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetY = rects[0].y;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            result.push_back(Rect{r.x, targetY, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> alignCenterVertical(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetCenter = rects[0].y + rects[0].height * 0.5;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            double newY = targetCenter - r.height * 0.5;
            result.push_back(Rect{r.x, newY, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> alignBottom(const std::vector<Rect>& rects) {
        if (rects.empty()) return {};
        double targetBottom = rects[0].y + rects[0].height;
        std::vector<Rect> result;
        result.reserve(rects.size());
        for (const Rect& r : rects) {
            double newY = targetBottom - r.height;
            result.push_back(Rect{r.x, newY, r.width, r.height});
        }
        return result;
    }

    static std::vector<Rect> distributeHorizontal(const std::vector<Rect>& rects) {
        if (rects.size() <= 2) return rects;

        double minX = rects.front().x;
        double maxRight = rects.back().x + rects.back().width;
        double totalWidth = 0.0;
        for (const Rect& r : rects) {
            totalWidth += r.width;
        }
        double spacing = (maxRight - minX - totalWidth) / (rects.size() - 1);

        std::vector<Rect> result;
        double currentX = minX;
        for (const Rect& r : rects) {
            result.push_back(Rect{currentX, r.y, r.width, r.height});
            currentX += r.width + spacing;
        }
        return result;
    }

    static std::vector<Rect> distributeVertical(const std::vector<Rect>& rects) {
        if (rects.size() <= 2) return rects;

        double minY = rects.front().y;
        double maxBottom = rects.back().y + rects.back().height;
        double totalHeight = 0.0;
        for (const Rect& r : rects) {
            totalHeight += r.height;
        }
        double spacing = (maxBottom - minY - totalHeight) / (rects.size() - 1);

        std::vector<Rect> result;
        double currentY = minY;
        for (const Rect& r : rects) {
            result.push_back(Rect{r.x, currentY, r.width, r.height});
            currentY += r.height + spacing;
        }
        return result;
    }
};

}