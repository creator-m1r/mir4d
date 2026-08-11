// MirUI/Core/Widget/WidgetType.hpp
// 📦 Расширенный список всех типов виджетов MirUI.
// Добавлены CheckBox, TextField, ComboBox, Slider, RadioButton, ProgressBar,
// Image, TableView, ScrollView, TabView.
//
// Чистый C++23, без платформенных зависимостей.


#pragma once

#include <cstdint>

namespace MirUI {

enum class WidgetType : uint32_t {
    // Контейнеры
    Window,
    Panel,
    DockPanel,
    Toolbar,
    Ribbon,
    Container,

    // Базовые виджеты
    Button,
    Label,
    TextField,
    CheckBox,
    ComboBox,
    Slider,

    // Сложные
    Tree,
    PropertyGrid,
    Viewport,
    Timeline,

    // Служебные
    Unknown
};

} // namespace MirUI