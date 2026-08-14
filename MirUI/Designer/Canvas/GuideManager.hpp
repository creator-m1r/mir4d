// MirUI/Designer/Canvas/GuideManager.hpp
// 📏 Менеджер направляющих линий — помогает выравнивать виджеты при перетаскивании.
//
// Когда ты тянешь кнопку мышкой по холсту, GuideManager вычисляет,
// где находятся края и центры других видимых виджетов, и если
// перетаскиваемый виджет оказывается близко к какой-либо линии,
// он «прилипает» к ней, а на экране появляется тонкая направляющая.
//
// Это работает так:
//   1. Перед началом перетаскивания холст вызывает generateGuides(),
//      передавая список всех виджетов, КРОМЕ того, который двигаем.
//   2. При каждом движении мыши холст вызывает snap(rect), который
//      проверяет, не приблизился ли какой-либо край перетаскиваемого
//      виджета к краю или центру другого виджета на расстояние меньше
//      порога (по умолчанию 4 пикселя).
//   3. Если приблизился — возвращает новый Rect с «прилипшими» координатами.
//   4. Одновременно запоминает, какие именно направляющие активны,
//      чтобы холст мог нарисовать их тонкими линиями.
//
// Направляющие НЕ изменяют дерево виджетов — они только влияют на
// предварительное отображение позиции во время перетаскивания.
// Когда пользователь отпускает мышь, окончательная позиция отправляется
// в C++ команду, и направляющие исчезают.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Rect.hpp"
#include "../../Core/Layout/Point.hpp"
#include "../../Core/Widget/Widget.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace MirUI {

// ── Типы направляющих линий ──────────────────────────────────
enum class GuideType {
    LeftEdge,           // левый край виджета
    RightEdge,          // правый край
    TopEdge,            // верхний край
    BottomEdge,         // нижний край
    HorizontalCenter,   // горизонтальный центр
    VerticalCenter      // вертикальный центр
};

// ── Одна направляющая ──────────────────────────────────────
struct Guide {
    GuideType type;
    double    position; // координата X (для вертикальных) или Y (для горизонтальных)
};

class GuideManager {
public:
    GuideManager()
        : m_snapThreshold(4.0)   // порог 4 пикселя — на таком расстоянии «прилипаем»
        , m_enabled(true)
    {}

    // ── Настройки ────────────────────────────────────────────
    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    // Видимость направляющих (синоним включённой привязки) — используется
    // PreviewManager для скрытия направляющих в режиме предпросмотра.
    void setVisible(bool visible) { m_enabled = visible; }
    [[nodiscard]] bool isVisible() const { return m_enabled; }

    void setSnapThreshold(double threshold) { m_snapThreshold = threshold; }
    [[nodiscard]] double snapThreshold() const { return m_snapThreshold; }

    // ── Генерация направляющих из списка виджетов ────────────
    // Принимает вектор указателей на виджеты (кроме того, который перетаскиваем).
    // Заполняет внутренний список направляющих на основе их bounds.
    void generateGuides(const std::vector<Widget*>& widgets) {
        m_guides.clear();
        for (const Widget* w : widgets) {
            if (!w || !w->isVisible()) continue;
            Rect bounds = w->bounds();

            // Вертикальные направляющие (совпадают с X координатами)
            m_guides.push_back({GuideType::LeftEdge, bounds.x});
            m_guides.push_back({GuideType::RightEdge, bounds.x + bounds.width});
            m_guides.push_back({GuideType::VerticalCenter, bounds.x + bounds.width * 0.5});

            // Горизонтальные направляющие (совпадают с Y координатами)
            m_guides.push_back({GuideType::TopEdge, bounds.y});
            m_guides.push_back({GuideType::BottomEdge, bounds.y + bounds.height});
            m_guides.push_back({GuideType::HorizontalCenter, bounds.y + bounds.height * 0.5});
        }

        // Убираем дубликаты (одинаковые позиции одного типа)
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

    // ── Привязка прямоугольника к направляющим ───────────────
    // Принимает прямоугольник (текущие bounds перетаскиваемого виджета).
    // Возвращает новый прямоугольник, чьи края/центры «прилипли»
    // к ближайшим направляющим (если расстояние меньше порога).
    // Также заполняет m_activeGuides — список направляющих, которые сработали.
    [[nodiscard]] Rect snap(const Rect& rect) {
        m_activeGuides.clear();
        if (!m_enabled) return rect;

        Rect result = rect;

        // Проверяем все возможные края перетаскиваемого виджета
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

    // ── Активные направляющие (для отрисовки) ────────────────
    [[nodiscard]] const std::vector<Guide>& activeGuides() const { return m_activeGuides; }

    // ── Очистка (после завершения перетаскивания) ────────────
    void clear() {
        m_guides.clear();
        m_activeGuides.clear();
    }

private:
    double m_snapThreshold;
    bool   m_enabled;

    std::vector<Guide> m_guides;        // все сгенерированные направляющие
    std::vector<Guide> m_activeGuides;  // те, которые сработали при последнем snap()

    // Вспомогательный метод: проверяет, нужно ли привязать значение edgeValue
    // к направляющим заданного типа, и если да — вызывает applyDelta.
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

} // namespace MirUI