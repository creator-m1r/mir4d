// MirUI/Core/Layout/LayoutData.hpp
// 📏 Данные компоновки для каждого виджета.
// Хранят предпочтения виджета по размерам: ширину, высоту,
// минимальные/максимальные ограничения, политику растяжения/сжатия,
// а также единицы измерения (пиксели, проценты, авто).
//
// Эти данные используются LayoutEngine для расчёта итоговых bounds,
// а также инспектором свойств в Designer, чтобы показывать
// поля ширины и высоты с выбором единиц измерения.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Size.hpp"   // наши Size, Point
#include <string>

namespace MirUI {

// ── Единицы измерения размеров ──────────────────────────────
// Когда пользователь вводит «300» в поле ширины, рядом выбирается
// единица: px, % или Auto. Это позволяет строить адаптивные интерфейсы.
enum class Unit {
    Pixel,   // абсолютная величина в логических пикселях
    Percent, // процент от размера родителя
    Auto     // размер определяется содержимым (fit)
};

// ── Политика изменения размера ──────────────────────────────
// Говорит LayoutEngine, как виджет должен вести себя при изменении
// размера родителя: оставаться фиксированным, заполнять свободное место,
// подстраиваться под содержимое или растягиваться.
enum class SizePolicy {
    Fixed,    // строго фиксированный размер (не меняется)
    Fill,     // занимает всё доступное пространство
    Fit,      // подстраивается под размер детей (контейнер)
    Stretch   // расширяется, но не сжимается меньше min
};

// ── Главная структура с данными компоновки ──────────────────
struct LayoutData {
    // Ширина и её единица
    double widthValue = 0.0;
    Unit   widthUnit  = Unit::Auto;   // если Auto, значение игнорируется

    // Высота и её единица
    double heightValue = 0.0;
    Unit   heightUnit  = Unit::Auto;

    // Минимальные и максимальные размеры (в пикселях).
    // Если не заданы, используются значения по умолчанию:
    // минимум = 0, максимум = очень большое число.
    Size minimumSize = { 0.0, 0.0 };
    Size maximumSize = { 1e9, 1e9 };

    // Политика растяжения по горизонтали и вертикали.
    SizePolicy horizontalPolicy = SizePolicy::Fixed;
    SizePolicy verticalPolicy   = SizePolicy::Fixed;

    // Внешние отступы (margin) и внутренние (padding) —
    // они хранятся в LayoutNode, но могут быть и здесь для Inspector'а.
    // Пока оставим только основные поля.

    // ── Вспомогательные методы ───────────────────────────────
    // Создать типичные данные для кнопки (фиксированный размер).
    static LayoutData fixed(double w, double h) {
        LayoutData ld;
        ld.widthValue = w;
        ld.widthUnit  = Unit::Pixel;
        ld.heightValue = h;
        ld.heightUnit  = Unit::Pixel;
        ld.horizontalPolicy = SizePolicy::Fixed;
        ld.verticalPolicy   = SizePolicy::Fixed;
        ld.minimumSize = {w, h};
        ld.maximumSize = {w, h};
        return ld;
    }

    // Создать данные для заполнения всего родителя (Fill).
    static LayoutData fill() {
        LayoutData ld;
        ld.horizontalPolicy = SizePolicy::Fill;
        ld.verticalPolicy   = SizePolicy::Fill;
        return ld;
    }

    // Создать данные для подстройки под содержимое (Fit).
    static LayoutData fit() {
        LayoutData ld;
        ld.widthUnit  = Unit::Auto;
        ld.heightUnit = Unit::Auto;
        ld.horizontalPolicy = SizePolicy::Fit;
        ld.verticalPolicy   = SizePolicy::Fit;
        return ld;
    }
};

} // namespace MirUI