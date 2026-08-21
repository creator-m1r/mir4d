#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool  MirUI_CAD_Initialize(void);
void  MirUI_CAD_Shutdown(void);
void  MirUI_CAD_Update(double deltaTime);
void  MirUI_CAD_Render(void);

void  MirUI_CAD_LoadModel(const char* path);
void  MirUI_CAD_SelectObject(const char* id);

#ifdef __cplusplus
}
#endif