#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void MirUI_Init(void);
void MirUI_Shutdown(void);

int64_t MirUI_AddWidget(const char* widgetType, double x, double y, double w, double h);
void MirUI_AddButton(const char* text, double x, double y, double w, double h);

void MirUI_MoveWidget(int64_t widgetId, double dx, double dy);
void MirUI_ResizeWidget(int64_t widgetId, double newWidth, double newHeight, double newX, double newY);
void MirUI_DeleteWidget(int64_t widgetId);

void MirUI_SetPropertyString(int64_t widgetId, const char* propertyName, const char* value);
void MirUI_SetPropertyDouble(int64_t widgetId, const char* propertyName, double value);
void MirUI_SetPropertyBool(int64_t widgetId, const char* propertyName, bool value);
const char* MirUI_GetPropertyString(int64_t widgetId, const char* propertyName);
void MirUI_SetProperty(int64_t widgetId, const char* propertyName, const char* value);

void MirUI_RenderFrame(void);
void MirUI_Undo(void);
void MirUI_Redo(void);

void MirUI_SelectWidget(int64_t widgetId);
void MirUI_ClearSelection(void);

void MirUI_CopyWidget(int64_t widgetId);
void MirUI_PasteWidget(int64_t parentId);
void MirUI_CutWidget(int64_t widgetId);

void MirUI_AlignWidgets(const int64_t* widgetIds, int count, const char* strategy);

void MirUI_NewProject(void);
bool MirUI_SaveProject(const char* path);
bool MirUI_LoadProject(const char* path);

void MirUI_EnterPreview(void);
void MirUI_ExitPreview(void);
void MirUI_TogglePreview(void);

void MirUI_SwitchTheme(const char* themeId);
const char* MirUI_CurrentThemeName(void);
void MirUI_RegisterTheme(const char* themeId);

const char* MirUI_GetThemeColor(const char* colorToken);
void MirUI_SetThemeColor(const char* colorToken, const char* hexColor);
double MirUI_GetThemeMetric(const char* metricToken);
void MirUI_SetThemeMetric(const char* metricToken, double value);
const char* MirUI_GetThemeFont(const char* fontToken);
void MirUI_SetThemeFont(const char* fontToken, const char* fontString);
double MirUI_GetThemeAnimationDuration(void);
void MirUI_SetThemeAnimationDuration(double duration);

const char* MirUI_GetWidgetStyleField(const char* widgetType, const char* widgetState, const char* fieldName);
void MirUI_SetWidgetStyleField(const char* widgetType, const char* widgetState, const char* fieldName, const char* value);

void MirUI_SwiftUI_UpdateViewNodes(const void* nodes, int count, int rootIndex);
void MirUI_ExecuteCommand(const char* commandId, int64_t widgetId);

#ifdef __cplusplus
}
#endif
