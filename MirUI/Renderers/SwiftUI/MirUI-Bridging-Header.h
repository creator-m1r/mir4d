// MirUI/Renderers/SwiftUI/MirUI-Bridging-Header.h
// 🌉 Обновлённый bridging-заголовок с функциями DesignerCore.
//
// Теперь SwiftUI может вызывать не только базовые операции,
// но и управление проектом (новый, сохранить, загрузить),
// предпросмотр, а также получать строковые представления свойств.
//
// Все функции реализованы в MirUICppBridge.mm.

#ifndef MIRUI_BRIDGING_HEADER_H
#define MIRUI_BRIDGING_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

// Инициализация и базовые операции
void MirUI_Init(void);
void MirUI_AddButton(const char* text, double x, double y, double w, double h);
void MirUI_RenderFrame(void);
void MirUI_Undo(void);
void MirUI_Redo(void);
void MirUI_Shutdown(void);

// Редактирование виджетов
void MirUI_MoveWidget(int64_t widgetId, double dx, double dy);
void MirUI_ResizeWidget(int64_t widgetId, double newWidth, double newHeight);
void MirUI_DeleteWidget(int64_t widgetId);

// Работа со свойствами
void MirUI_SetPropertyString(int64_t widgetId, const char* propertyName, const char* value);
void MirUI_SetPropertyDouble(int64_t widgetId, const char* propertyName, double value);
void MirUI_SetPropertyBool(int64_t widgetId, const char* propertyName, bool value);
const char* MirUI_GetPropertyString(int64_t widgetId, const char* propertyName);
void MirUI_SetProperty(int64_t widgetId, const char* propertyName, const char* value);

// Выделение
void MirUI_SelectWidget(int64_t widgetId);
void MirUI_ClearSelection(void);

// Буфер обмена
void MirUI_CopyWidget(int64_t widgetId);
void MirUI_PasteWidget(int64_t parentId);
void MirUI_CutWidget(int64_t widgetId);

// Выравнивание
void MirUI_AlignWidgets(const int64_t* widgetIds, int count, const char* strategy);

// Управление проектом
void MirUI_NewProject(void);
bool MirUI_SaveProject(const char* path);
bool MirUI_LoadProject(const char* path);

// Предпросмотр
void MirUI_EnterPreview(void);
void MirUI_ExitPreview(void);
void MirUI_TogglePreview(void);

// Передача данных в SwiftUI
void MirUI_SwiftUI_UpdateViewNodes(const void* nodes, int count, int rootIndex);
void MirUI_ExecuteCommand(const char* commandId, int64_t widgetId);

#ifdef __cplusplus
}
#endif

#endif // MIRUI_BRIDGING_HEADER_H