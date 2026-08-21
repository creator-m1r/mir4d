// MirUI/Designer/Canvas/AlignmentManager.hpp
// 📐 Менеджер выравнивания виджетов.
//
// Когда на холсте выделено несколько кнопок, пользователь может захотеть
// выровнять их по левому краю, по центру, по правому краю или равномерно
// распределить. AlignmentManager содержит статические методы, которые
// по набору прямоугольников (Rect) вычисляют новые позиции, следуя
// выбранной стратегии выравнивания.
//
// Важно: AlignmentManager не изменяет дерево виджетов и не создаёт команд.
// Он только вычисляет новые значения bounds. Применение этих значений
// (создание MoveWidgetCommand или ResizeWidgetCommand) остаётся за
// вызывающим кодом — холстом или мостом. Так мы сохраняем чистоту
// разделения: геометрические расчёты отдельно, выполнение команд отдельно.
//
// Поддерживаемые стратегии выравнивания (enum AlignStrategy):
//   • Left              — по левому краю первого выделенного
//   • CenterHorizontal  — горизонтально по центру первого
//   • Right             — по правому краю первого
//   • Top               — по верхнему краю первого
//   • CenterVertical    — вертикально по центру первого
//   • Bottom            — по нижнему краю первого
//   • DistributeHorizontal — равномерно распределить по горизонтали
//   • DistributeVertical   — равномерно распределить по вертикали
//
// Пример использования (в мосте или холсте):
//   1. Собрать Rect всех выделенных виджетов.
//   2. Вызвать AlignmentManager::align(rects, AlignStrategy::Left).
//   3. Для каждого виджета сравнить старый и новый Rect,
//      создать ResizeWidgetCommand (которая умеет менять и позицию) и выполнить.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/Rect.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

namespace MirUI {

// ── Стратегии выравнивания ──────────────────────────────────
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
    // ── Главный метод выравнивания ──────────────────────────
    // Принимает вектор прямоугольников (bounds виджетов) и стратегию.
    // Возвращает новый вектор прямоугольников с изменёнными позициями,
    // но с сохранением размеров каждого виджета.
    // Порядок прямоугольников сохраняется.
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
        return rects; // неизвестная стратегия — возвращаем как есть
    }

private:
    // ── Выравнивание по левому краю ─────────────────────────
    // Все прямоугольники получают координату X, равную X первого прямоугольника.
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

    // ── Выравнивание по горизонтальному центру ──────────────
    // Центр каждого прямоугольника по X становится равным центру первого.
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

    // ── Выравнивание по правому краю ────────────────────────
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

    // ── Выравнивание по верхнему краю ───────────────────────
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

    // ── Выравнивание по вертикальному центру ────────────────
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

    // ── Выравнивание по нижнему краю ────────────────────────
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

    // ── Равномерное распределение по горизонтали ────────────
    // Сохраняем левый край первого и правый край последнего,
    // остальные равномерно размещаем между ними.
    static std::vector<Rect> distributeHorizontal(const std::vector<Rect>& rects) {
        if (rects.size() <= 2) return rects; // нечего распределять

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

    // ── Равномерное распределение по вертикали ──────────────
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

} // namespace MirUI