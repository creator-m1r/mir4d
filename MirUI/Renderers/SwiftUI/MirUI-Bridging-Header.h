
#ifndef MIRUI_BRIDGING_HEADER_H
#define MIRUI_BRIDGING_HEADER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void MirUI_Init(void);
int64_t MirUI_AddWidget(const char* widgetType, double x, double y, double w, double h);
void MirUI_AddButton(const char* text, double x, double y, double w, double h);
void MirUI_Shutdown(void);

void MirUI_SetProperty(int64_t widgetId, const char* propertyName, const char* value);
void MirUI_SelectWidget(int64_t widgetId);
void MirUI_ClearSelection(void);

bool MirUI_SaveProject(const char* path);
bool MirUI_LoadProject(const char* path);

void MirUI_EnterPreview(void);
void MirUI_ExitPreview(void);
void MirUI_TogglePreview(void);

#ifdef __cplusplus
}
#endif

#endif
