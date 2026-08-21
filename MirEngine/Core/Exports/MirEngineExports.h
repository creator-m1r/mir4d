#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t width;
    uint32_t height;
} MirEngineSize2D;

void* MirEngineCreateMacOpenGLContext(
    void* view,
    MirEngineSize2D size
);

void MirEngineDestroyOpenGLContext(
    void* context
);

void MirEngineSetOpenGLContextView(
    void* context,
    void* view
);

void* MirEngineCreateOpenGLRenderer(
    void* context
);

bool MirEngineInitializeRenderer(
    void* renderer
);

void MirEngineDestroyRenderer(
    void* renderer
);

void MirEngineSetPlanes(
    void* renderer,
    int count,
    const uint32_t* ids,
    const float* origins,
    const float* normals,
    const float* xAxes,
    const float* yAxes,
    const float* colors,
    const float* sizes,
    const bool* active,
    const bool* selected
);

void MirEngineSetCursor(void* renderer, float ndcX, float ndcY, bool active);

void MirEngineSetSketch(
    void* renderer,
    int segmentCount,
    const float* ax,
    const float* ay,
    const float* bx,
    const float* by,
    const float* colors,
    const float* origin,
    const float* xAxis,
    const float* yAxis
);

void* MirEngineCreatePlaneStore(void);
void MirEngineDestroyPlaneStore(void* store);
void MirEnginePlaneStoreAddBasePlanes(void* store);
uint32_t MirEnginePlaneStoreCreateOffsetPlane(void* store,
                                              uint32_t basePlane,
                                              double offset,
                                              double angleDeg);
int MirEnginePlaneStoreSnapshot(void* store,
                                int maxCount,
                                uint32_t* ids,
                                float* origins,
                                float* normals,
                                float* xAxes,
                                float* yAxes,
                                float* colors,
                                float* sizes,
                                bool* active,
                                bool* selected);

void* MirEngineCreateViewport(
    void* renderer,
    uint32_t width,
    uint32_t height
);

void MirEngineDestroyViewport(
    void* viewport
);

void MirEngineResize(
    void* viewport,
    uint32_t width,
    uint32_t height
);

void MirEngineRender(
    void* viewport
);

void MirEngineGetCameraOrientation(
    void* viewport,
    float* theta,
    float* phi,
    float* distance
);

void MirEngineSetCameraOrientation(
    void* viewport,
    float theta,
    float phi,
    float distance
);

void MirEngineSetCameraProjection(
    void* viewport,
    int projection
);

int MirEngineGetCameraProjection(
    void* viewport
);

void MirEngineSetCameraFov(
    void* viewport,
    float fovYRadians
);

void MirEngineSetActiveCameraPreset(
    void* viewport,
    int preset
);

void MirEngineGetCameraPresetOrientation(
    int preset,
    float* theta,
    float* phi,
    float* distance
);

void MirEngineFitViewport(
    void* viewport
);

void MirEngineViewportMouseDown(
    void* viewport,
    int button,
    float x,
    float y
);

void MirEngineViewportMouseUp(
    void* viewport,
    int button,
    float x,
    float y
);

void MirEngineViewportMouseMove(
    void* viewport,
    float x,
    float y
);

void MirEngineViewportScroll(
    void* viewport,
    float delta
);

void MirEngineViewportZoomAt(
    void* viewport,
    float delta,
    float x,
    float y
);

void MirEngineViewportPan(
    void* viewport,
    float dx,
    float dy
);

void MirEngineViewportOrbit(
    void* viewport,
    float dx,
    float dy
);

void MirEngineViewportClick(
    void* viewport,
    float x,
    float y,
    bool addToSelection
);

void MirEngineViewportHover(
    void* viewport,
    float x,
    float y
);

void MirEngineViewportHoverClear(
    void* viewport
);

uint64_t MirEngineGetSelectedObjectId(
    void* viewport
);

bool MirEngineDeformSelected(
    void* viewport,
    double x, double y, double z,
    double radius,
    double strength,
    int mode
);

bool MirEngineBeginDeformSelected(
    void* viewport
);

bool MirEngineEndDeformSelected(
    void* viewport
);

bool MirEngineDeleteSelectedObject(
    void* viewport
);

void MirEngineClearSelection(
    void* viewport
);

void MirEngineSelectObject(
    void* viewport,
    uint64_t objectId
);

bool MirEnginePickWorldPoint(
    void* viewport,
    double nx, double ny,
    double* outX, double* outY, double* outZ,
    uint64_t* outObjectId
);

void MirEngineViewportDragCancel(
    void* viewport
);

typedef struct
{
    double px, py, pz;
    double qx, qy, qz, qw;
    double sx, sy, sz;
} MirTransform;

bool MirEnginePickHandRay(
    void* viewport,
    double ox, double oy, double oz,
    double dx, double dy, double dz,
    uint64_t* outObjectId,
    double* outDistance
);

void MirEngineBeginGrab(
    void* viewport,
    uint64_t objectId
);

bool MirEnginePreviewGrab(
    void* viewport,
    uint64_t objectId,
    MirTransform transform
);

bool MirEngineCommitGrab(
    void* viewport,
    uint64_t objectId
);

void MirEngineCancelGrab(
    void* viewport
);

bool MirEngineGetObjectTransform(
    void* viewport,
    uint64_t objectId,
    MirTransform* outTransform
);

void MirEngineSetHandHover(
    void* viewport,
    uint64_t objectId
);

void MirEngineSetHandSkeleton(
    void* viewport,
    int32_t mode,
    int32_t handCount,
    const double* positions,
    const double* confidence,
    const int32_t* handedness,
    const double* pinch,
    const int32_t* gesture
);

void MirEngineSetHandSkeletonStyle(
    void* viewport,
    float leftR, float leftG, float leftB,
    float rightR, float rightG, float rightB,
    float jointSize, float tipSize, float wristSize,
    float alpha, int32_t depthTest
);

void MirEngineSetHandSkeletonTopology(
    void* viewport,
    int32_t boneCount,
    const int32_t* bones
);

void MirEngineClearHandSkeleton(void* viewport);

bool MirEngineGetCameraEye(
    void* viewport,
    double* outX,
    double* outY,
    double* outZ
);

bool MirEngineUndo(
    void* viewport
);

bool MirEngineRedo(
    void* viewport
);

bool MirEngineCanUndo(
    void* viewport
);

bool MirEngineCanRedo(
    void* viewport
);

bool MirEngineCreateBox(
    void* viewport,
    double width,
    double depth,
    double height,
    uint64_t* objectId
);

bool MirEngineGetSelectedObjectMetrics(
    void* viewport,
    char* outJson,
    size_t outCapacity
);

bool MirEngineImportMesh(
    void* viewport,
    const char* path
);

bool MirEngineExportStl(
    void* viewport,
    const char* path,
    bool selectionOnly
);

bool MirEngineImportStep(
    void* viewport,
    const char* path
);

bool MirEngineImportStepBRep(
    void* viewport,
    const char* path
);

bool MirEngineExportStepBRep(
    void* viewport,
    const char* path,
    bool selectionOnly
);

bool MirEngineExportStep(
    void* viewport,
    const char* path,
    bool selectionOnly
);

int32_t MirEngineMaterialCount(void);

bool MirEngineMaterialName(
    int32_t materialId,
    char* buffer,
    size_t bufferSize
);

bool MirEngineSetObjectMaterial(
    void* viewport,
    uint64_t objectId,
    int32_t materialId
);

bool MirEngineGetOpenGLDiagnostics(
    void* renderer,
    char* buffer,
    size_t bufferSize
);

typedef struct MirEngineSketchDocument MirEngineSketchDocument;

typedef enum MirEngineSketchConstraint
{
    MirEngineSketchCoincident = 0,
    MirEngineSketchHorizontal,
    MirEngineSketchVertical,
    MirEngineSketchParallel,
    MirEngineSketchPerpendicular,
    MirEngineSketchTangent,
    MirEngineSketchConcentric,
    MirEngineSketchEqual,
    MirEngineSketchSymmetric,
    MirEngineSketchDistance,
    MirEngineSketchAngle,
    MirEngineSketchRadius,
    MirEngineSketchDiameter
} MirEngineSketchConstraint;

void* MirEngineSketchCreateDocument(void);
void MirEngineSketchDestroyDocument(void* doc);
uint32_t MirEngineSketchAddLine(void* doc, float x1, float y1, float x2, float y2);
uint32_t MirEngineSketchAddCircle(void* doc, float cx, float cy, float r);
uint32_t MirEngineSketchAddConstraint(void* doc, int type, uint32_t g1, uint32_t g2, double value);
bool MirEngineSketchSolve(void* doc);
bool MirEngineSketchGetLine(void* doc, uint32_t id, float* x1, float* y1, float* x2, float* y2);
bool MirEngineSketchGetCircle(void* doc, uint32_t id, float* cx, float* cy, float* r);
uint32_t MirEngineSketchAddArc(void* doc, float cx, float cy, float r, float startAngle, float endAngle);
uint32_t MirEngineSketchAddSpline(void* doc, const float* xs, const float* ys, uint32_t count, bool closed);
uint32_t MirEngineSketchSplineCount(void* doc);
bool MirEngineSketchSplineAt(void* doc, uint32_t index, float* xs, float* ys, uint32_t* count, bool* closed);
uint32_t MirEngineSketchGeometryCount(void* doc);
int MirEngineSketchGeometryTypeAt(void* doc, uint32_t index);
bool MirEngineSketchLineAt(void* doc, uint32_t index, float* x1, float* y1, float* x2, float* y2);
bool MirEngineSketchArcAt(void* doc, uint32_t index, float* cx, float* cy, float* r, float* sa, float* ea);
bool MirEngineSketchCircleAt(void* doc, uint32_t index, float* cx, float* cy, float* r);
bool MirEngineSketchRemoveGeometry(void* doc, uint32_t id);
void MirEngineSketchClear(void* doc);
uint32_t MirEngineSketchGeometryIdAt(void* doc, uint32_t index);
void MirEngineSketchSetPlane(void* doc, uint32_t planeId,
                             float ox, float oy, float oz,
                             float nx, float ny, float nz,
                             float xx, float xy, float xz,
                             float yx, float yy, float yz);
bool MirEngineSketchRemoveConstraint(void* doc, uint32_t id);
uint32_t MirEngineSketchConstraintCount(void* doc);
bool MirEngineSketchConstraintAt(void* doc, uint32_t index, int32_t* type, uint32_t* g1, uint32_t* g2, double* value);

const char* MirEngineGetLastError(
    void* viewport
);

#ifdef __cplusplus
}
#endif
