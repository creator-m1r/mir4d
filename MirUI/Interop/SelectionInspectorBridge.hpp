#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MirUISelectionInspectorHandle MirUISelectionInspectorHandle;

MirUISelectionInspectorHandle* mirui_selection_inspector_create(void);
void mirui_selection_inspector_destroy(MirUISelectionInspectorHandle* handle);

void mirui_selection_inspector_clear(MirUISelectionInspectorHandle* handle);

void mirui_selection_inspector_update_face(
    MirUISelectionInspectorHandle* handle,
    std::uint64_t id,
    std::uint64_t triangleCount,
    double area,
    double centerX,
    double centerY,
    double centerZ,
    double normalX,
    double normalY,
    double normalZ);

std::size_t mirui_selection_inspector_property_count(
    const MirUISelectionInspectorHandle* handle);

const char* mirui_selection_inspector_property_name(
    const MirUISelectionInspectorHandle* handle,
    std::size_t index);

const char* mirui_selection_inspector_property_value(
    const MirUISelectionInspectorHandle* handle,
    std::size_t index);

#ifdef __cplusplus
}
#endif
