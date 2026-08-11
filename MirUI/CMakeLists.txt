# CMakeLists.txt — корневой файл сборки проекта MirUI (исправленный).
# ================================================================
# Исправлены пути к заголовкам: убрана лишняя вложенность "MirUI/".
# Теперь все файлы находятся в корне репозитория, и CMake их видит.
#
# Сборка:
#   cmake -B build
#   cmake --build build
#   ./build/bin/mirui_example
#
# Чистый C++23, без внешних зависимостей.

cmake_minimum_required(VERSION 3.20)
project(MirUI
    VERSION 0.2.0
    DESCRIPTION "Кроссплатформенный UI-фреймворк на C++23"
    LANGUAGES CXX
)

# ── Стандарт C++ ────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Опции сборки ────────────────────────────────────────────
option(MIRUI_BUILD_EXAMPLES "Собирать примеры" ON)

# ── Главная библиотека MirUI (header-only) ──────────────────
add_library(MirUI INTERFACE)
target_include_directories(MirUI INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Перечисляем все созданные нами заголовки (и исходники)
set(MIRUI_HEADERS
    # Публичная точка входа
    MirUI.hpp

    # Foundation
    Foundation/Color/Color.hpp
    Foundation/Color/ColorPalette.hpp
    Foundation/Metrics/Metrics.hpp
    Foundation/Animation/Animation.hpp
    Foundation/Animation/AnimationSpec.hpp
    Foundation/Typography/Font.hpp
    Foundation/Typography/Typography.hpp
    Foundation/Icons/IconID.hpp

    # Core/Widget
    Core/Widget/WidgetID.hpp
    Core/Widget/WidgetType.hpp
    Core/Widget/Widget.hpp
    Core/Widget/Widget.cpp
    Core/Widget/WidgetTree.hpp
    Core/Widget/WidgetTree.cpp
    Core/Widget/PropertyType.hpp
    Core/Widget/PropertyDescriptor.hpp
    Core/Widget/WidgetFactory.hpp

    # Core/Layout
    Core/Layout/Point.hpp
    Core/Layout/Size.hpp
    Core/Layout/Rect.hpp
    Core/Layout/Insets.hpp
    Core/Layout/LayoutConstraints.hpp
    Core/Layout/LayoutNode.hpp
    Core/Layout/LayoutEngine.hpp
    Core/Layout/Unit.hpp
    Core/Layout/LayoutData.hpp
    Core/Layout/Transform.hpp

    # Core/Theme
    Core/Theme/Theme.hpp
    Core/Theme/ThemeManager.hpp

    # Core/State
    Core/State/StateValue.hpp
    Core/State/StateKey.hpp
    Core/State/StateStore.hpp
    Core/State/PropertyValue.hpp

    # Core/Commands
    Core/Commands/CommandID.hpp
    Core/Commands/Command.hpp
    Core/Commands/CommandContext.hpp
    Core/Commands/CommandBus.hpp
    Core/Commands/CommandHistory.hpp

    # Core/Events
    Core/Events/EventType.hpp
    Core/Events/Event.hpp
    Core/Events/EventDispatcher.hpp

    # Core/Focus
    Core/Focus/FocusManager.hpp

    # Core/Rendering
    Core/Rendering/Renderer.hpp
    Core/Rendering/RenderCommand.hpp
    Core/Rendering/RenderCommandBuffer.hpp

    # Core/Selection
    Core/Selection/SelectionManager.hpp

    # Widgets
    Widgets/Button/Button.hpp
    Widgets/Label/Label.hpp
    Widgets/Toolbar/Toolbar.hpp
    Widgets/Tree/TreeNode.hpp
    Widgets/Tree/Tree.hpp
    Widgets/PropertyGrid/Property.hpp
    Widgets/PropertyGrid/PropertyGrid.hpp
    Widgets/Viewport/Viewport.hpp

    # Workspace
    Workspace/PanelManager/PanelDescriptor.hpp
    Workspace/PanelManager/PanelManager.hpp
    Workspace/WorkspaceManager/Workspace.hpp
    Workspace/WorkspaceManager/WorkspaceManager.hpp
    Workspace/LayoutManager/LayoutManager.hpp
    Workspace/WindowManager/WindowDescriptor.hpp
    Workspace/WindowManager/WindowManager.hpp

    # Designer/Document
    Designer/Document/UIDocument.hpp
    Designer/Document/UIFormatVersion.hpp
    Designer/Document/UIReader.hpp
    Designer/Document/UIWriter.hpp

    # Designer/Commands
    Designer/Commands/AddWidgetCommand.hpp
    Designer/Commands/DeleteWidgetCommand.hpp
    Designer/Commands/MoveWidgetCommand.hpp
    Designer/Commands/ResizeWidgetCommand.hpp
    Designer/Commands/ChangePropertyCommand.hpp
    Designer/Commands/RenameWidgetCommand.hpp

    # Designer/Canvas
    Designer/Canvas/DesignerCanvas.hpp
    Designer/Canvas/DragController.hpp
    Designer/Canvas/GridManager.hpp
    Designer/Canvas/GuideManager.hpp
    Designer/Canvas/ResizeController.hpp

    # Designer/History
    Designer/History/DesignerHistory.hpp

    # Designer/Inspector
    Designer/Inspector/ColorEditor.hpp
    Designer/Inspector/EnumEditor.hpp
    Designer/Inspector/FontEditor.hpp
    Designer/Inspector/InspectorModel.hpp
    Designer/Inspector/PropertyEditor.hpp
    Designer/Inspector/SizeEditor.hpp

    # Designer/Preview
    Designer/Preview/PreviewManager.hpp
    Designer/Preview/PreviewMode.hpp

    # Designer/Themes
    Designer/Themes/ColorTokenEditor.hpp
    Designer/Themes/MetricsEditor.hpp
    Designer/Themes/ThemeEditor.hpp
    Designer/Themes/TypographyEditor.hpp

    # Designer/Toolbox
    Designer/Toolbox/Toolbox.hpp
    Designer/Toolbox/ToolboxItem.hpp
    Designer/Toolbox/ToolboxModel.hpp

    # Designer/Application
    Designer/Application/DesignerApplication.hpp
    Designer/Application/DesignerState.hpp

    # Schema
    Schema/CommandSchema.hpp
    Schema/EventSchema.hpp
    Schema/LayoutSchema.hpp
    Schema/PropertySchema.hpp
    Schema/ThemeSchema.hpp
    Schema/WidgetSchema.hpp

    # Renderers
    Renderers/NullRenderer.hpp
)

target_sources(MirUI INTERFACE ${MIRUI_HEADERS})

# ── Пример использования ────────────────────────────────────
if(MIRUI_BUILD_EXAMPLES)
    add_executable(mirui_example
        examples/main.cpp
    )
    target_link_libraries(mirui_example PRIVATE MirUI)
    target_compile_features(mirui_example PRIVATE cxx_std_23)

    set_target_properties(mirui_example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# ── Информация при конфигурации ─────────────────────────────
message(STATUS "================================================")
message(STATUS "  MirUI v${PROJECT_VERSION}")
message(STATUS "  Стандарт: C++23")
message(STATUS "  Header-only библиотека с примерами")
message(STATUS "================================================")