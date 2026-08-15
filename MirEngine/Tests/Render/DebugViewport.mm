// MirEngine/Tests/Render/DebugViewport.mm
// =================================================================================
// Автономный отладочный стенд viewport'а MIR 4D (вне MirUI / SwiftUI).
//
// Открывает собственное NSWindow с NSView, создаёт OpenGL контекст, renderer и
// viewport ровно тем же путём C ABI (MirEngineExports.h), которым пользуется
// MirUI, и позволяет:
//   * управлять камерой мышью и клавиатурой;
//   * изучать состояние камеры (theta/phi/distance, eye, near/far, FPS);
//   * печатать диагностику OpenGL и ошибки движка;
//   * создавать примитивы и проверять выбор (pick).
//
// Управление:
//   ЛКМ drag         — орбита (как в MirUI)
//   ПКМ / средняя    — панорама
//   scroll           — зум
//   Стрелки          — орбита фиксированным шагом
//   + / -            — зум фиксированным шагом
//   1..6             — пресеты камеры (front/back/left/right/top/bottom/isometric)
//   F                — Fit viewport
//   I                — состояние камеры (theta/phi/distance, eye, near/far)
//   D                — диагностика OpenGL
//   E                — последняя ошибка движка
//   B                — создать куб 2x2x2
//   C                — клик (выбор) в текущей точке мыши
//   Q                — выход
//
// Режим --smoke: без показа окна рендерит N кадров (по умолчанию 120) и
// печатает состояние камеры на дистанциях 12/50/200/1000 — для проверки
// обрезки геометрии в консоли.
// =================================================================================

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

#include "MirEngine/Core/Exports/MirEngineExports.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{

void* gRenderer = nullptr;
void* gViewport = nullptr;
NSView* gView = nullptr;
bool gQuit = false;
bool gSmoke = false;
int gSmokeFrames = 120;
bool gAutoShot = false;

NSPoint gLastMouse = NSZeroPoint;

// Совпадает с Camera::position() в MirEngine/Viewport/Camera.hpp.
void computeEye(double theta, double phi, double distance,
                double& outX, double& outY, double& outZ)
{
    const double sinPhi = std::sin(phi);
    const double cosPhi = std::cos(phi);
    const double sinTheta = std::sin(theta);
    const double cosTheta = std::cos(theta);

    outX = distance * sinPhi * sinTheta;
    outY = distance * cosPhi;
    outZ = distance * sinPhi * cosTheta;
}

// Повторяет алгоритм near/far из ViewportRuntime::render() для куба 2x2x2
// (sceneRadius = диагональ/2 ≈ 1.732), нацеленного в центр сцены.
void computeClipping(double distance, double& nearPlane, double& farPlane)
{
    const double sceneRadius = std::sqrt(3.0);
    const double distanceToScene = distance;

    farPlane = distanceToScene + sceneRadius +
               std::max(sceneRadius * 0.5, distanceToScene * 0.1) + 1.0;
    nearPlane = std::clamp(distanceToScene * 0.0008, 0.0005, 5.0);
}

void printCameraInfo()
{
    float theta = 0.0f, phi = 0.0f, distance = 0.0f;
    MirEngineGetCameraOrientation(gViewport, &theta, &phi, &distance);

    double eyeX = 0.0, eyeY = 0.0, eyeZ = 0.0;
    computeEye(theta, phi, distance, eyeX, eyeY, eyeZ);

    double nearPlane = 0.0, farPlane = 0.0;
    computeClipping(distance, nearPlane, farPlane);

    printf("[camera] theta=%9.4f rad (%7.2f deg)  phi=%9.4f rad (%7.2f deg)  "
           "distance=%10.3f\n",
           static_cast<double>(theta), theta * 180.0 / M_PI,
           static_cast<double>(phi), phi * 180.0 / M_PI,
           static_cast<double>(distance));
    printf("[camera] eye   = (%10.4f, %10.4f, %10.4f)\n", eyeX, eyeY, eyeZ);
    printf("[camera] near  = %10.6f  far = %12.3f  (log10 far/near = %.3f)\n",
           nearPlane, farPlane, std::log10(farPlane / nearPlane));
}

void printOpenGLDiagnostics()
{
    char buffer[4096];
    if (MirEngineGetOpenGLDiagnostics(gRenderer, buffer, sizeof(buffer)))
        printf("[opengl]\n%s\n", buffer);
    else
        printf("[opengl] diagnostics unavailable\n");
}

void printLastError()
{
    const char* error = MirEngineGetLastError(gViewport);
    printf("[engine] last error: %s\n", error ? error : "(none)");
}

void printHelp()
{
    printf("MIR 4D Debug Viewport\n");
    printf("  ЛКМ drag        — орбита\n");
    printf("  ПКМ / средняя   — панорама\n");
    printf("  scroll          — зум\n");
    printf("  Стрелки         — орбита шагом\n");
    printf("  + / -           — зум шагом\n");
    printf("  1..6            — пресеты камеры\n");
    printf("  F               — fit viewport\n");
    printf("  I               — состояние камеры\n");
    printf("  D               — диагностика OpenGL\n");
    printf("  E               — последняя ошибка движка\n");
    printf("  B               — создать куб 2x2x2\n");
    printf("  C               — клик (выбор) в точке мыши\n");
    printf("  Q               — выход\n\n");
}

void renderFrame()
{
    if (gViewport)
        MirEngineRender(gViewport);
}

// Захват текущего кадра через glReadPixels и сохранение в PPM.
// Требует текущего GL context (вызывается на главном потоке).
void saveFrameAsPPM(const char* path)
{
    NSRect backing = [gView convertRectToBacking:gView.bounds];
    const int width = (int)std::max(backing.size.width, 1.0);
    const int height = (int)std::max(backing.size.height, 1.0);

    std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    FILE* file = std::fopen(path, "wb");
    if (!file)
    {
        printf("[capture] failed to open %s\n", path);
        return;
    }
    std::fprintf(file, "P6\n%d %d\n255\n", width, height);
    std::fwrite(pixels.data(), 1, pixels.size(), file);
    std::fclose(file);
    printf("[capture] saved %dx%d frame to %s\n", width, height, path);
}

bool createBox()
{
    uint64_t objectId = 0;
    const bool ok = MirEngineCreateBox(gViewport, 2.0, 2.0, 2.0, &objectId);
    printf("[box] create result=%s objectId=%llu\n",
           ok ? "true" : "false",
           static_cast<unsigned long long>(objectId));
    if (!ok)
        printLastError();
    return ok;
}

} // namespace

// =================================================================================
// Отладочный NSView — обработка мыши и клавиатуры, путь координат как в MirUI:
// convert(event.locationInWindow, from: nil) * backingScaleFactor
// =================================================================================

@interface MIR4DDebugView : NSView
@end

@implementation MIR4DDebugView

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event
{
    (void)event;
    return YES;
}

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];
    NSTrackingArea* area = [[NSTrackingArea alloc]
        initWithRect:NSZeroRect
             options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
               owner:self
            userInfo:nil];
    [self addTrackingArea:area];
}

- (void)resizeToEngine
{
    const NSRect bounds = [self convertRectToBacking:[self bounds]];
    MirEngineResize(gViewport,
                    static_cast<uint32_t>(std::max(bounds.size.width, 1.0)),
                    static_cast<uint32_t>(std::max(bounds.size.height, 1.0)));
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    if (gViewport)
        [self resizeToEngine];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    if (gViewport)
        [self resizeToEngine];
}

- (NSPoint)enginePoint:(NSEvent*)event
{
    const NSPoint local = [self convertPoint:event.locationInWindow fromView:nil];
    const CGFloat scale = [self.window backingScaleFactor];
    return NSMakePoint(local.x * scale, local.y * scale);
}

// ------------------------------------------------------------
// Mouse
// ------------------------------------------------------------

- (void)forwardMouseButton:(NSEvent*)event down:(BOOL)down
{
    const NSPoint point = [self enginePoint:event];
    if (down)
        MirEngineViewportMouseDown(gViewport, (int32_t)event.buttonNumber,
                                   (float)point.x, (float)point.y);
    else
        MirEngineViewportMouseUp(gViewport, (int32_t)event.buttonNumber,
                                 (float)point.x, (float)point.y);
}

- (void)mouseDown:(NSEvent*)event
{
    [self.window makeFirstResponder:self];
    gLastMouse = [self enginePoint:event];
    [self forwardMouseButton:event down:YES];
}

- (void)mouseUp:(NSEvent*)event
{
    [self forwardMouseButton:event down:NO];
    const NSPoint point = [self enginePoint:event];
    const double dx = point.x - gLastMouse.x;
    const double dy = point.y - gLastMouse.y;
    if (dx * dx + dy * dy <= 9.0 && event.buttonNumber == 0)
    {
        MirEngineViewportClick(gViewport, (float)point.x, (float)point.y, false);
        const uint64_t selected = MirEngineGetSelectedObjectId(gViewport);
        printf("[pick] clicked at (%.1f, %.1f), selected id = %llu\n",
               (double)point.x, (double)point.y,
               static_cast<unsigned long long>(selected));
    }
}

- (void)rightMouseDown:(NSEvent*)event
{
    [self forwardMouseButton:event down:YES];
}

- (void)rightMouseUp:(NSEvent*)event
{
    [self forwardMouseButton:event down:NO];
}

- (void)otherMouseDown:(NSEvent*)event
{
    [self forwardMouseButton:event down:YES];
}

- (void)otherMouseUp:(NSEvent*)event
{
    [self forwardMouseButton:event down:NO];
}

- (void)mouseDragged:(NSEvent*)event
{
    const NSPoint point = [self enginePoint:event];
    MirEngineViewportMouseMove(gViewport, (float)point.x, (float)point.y);
}

- (void)rightMouseDragged:(NSEvent*)event
{
    const NSPoint point = [self enginePoint:event];
    MirEngineViewportMouseMove(gViewport, (float)point.x, (float)point.y);
}

- (void)otherMouseDragged:(NSEvent*)event
{
    const NSPoint point = [self enginePoint:event];
    MirEngineViewportMouseMove(gViewport, (float)point.x, (float)point.y);
}

- (void)mouseMoved:(NSEvent*)event
{
    const NSPoint point = [self enginePoint:event];
    MirEngineViewportMouseMove(gViewport, (float)point.x, (float)point.y);
}

- (void)scrollWheel:(NSEvent*)event
{
    MirEngineViewportScroll(gViewport, (float)event.scrollingDeltaY);
}

// ------------------------------------------------------------
// Keyboard
// ------------------------------------------------------------

- (void)keyDown:(NSEvent*)event
{
    const NSString* chars = event.charactersIgnoringModifiers;
    if (chars.length == 0)
        return;

    const unichar key = [chars characterAtIndex:0];

    if (key == 'q' || key == 'Q' || key == 27)
    {
        gQuit = true;
        return;
    }

    if (key == NSUpArrowFunctionKey || key == NSDownArrowFunctionKey ||
        key == NSLeftArrowFunctionKey || key == NSRightArrowFunctionKey)
    {
        const float dx = (key == NSRightArrowFunctionKey) ? 0.08f
                        : (key == NSLeftArrowFunctionKey) ? -0.08f : 0.0f;
        const float dy = (key == NSUpArrowFunctionKey) ? 0.08f
                        : (key == NSDownArrowFunctionKey) ? -0.08f : 0.0f;
        MirEngineViewportOrbit(gViewport, dx, dy);
        return;
    }

    switch (key)
    {
    case '+':
    case '=':
        MirEngineViewportScroll(gViewport, 1.0f);
        break;
    case '-':
    case '_':
        MirEngineViewportScroll(gViewport, -1.0f);
        break;
    case '1': MirEngineSetActiveCameraPreset(gViewport, 0); break;
    case '2': MirEngineSetActiveCameraPreset(gViewport, 1); break;
    case '3': MirEngineSetActiveCameraPreset(gViewport, 2); break;
    case '4': MirEngineSetActiveCameraPreset(gViewport, 3); break;
    case '5': MirEngineSetActiveCameraPreset(gViewport, 4); break;
    case '6': MirEngineSetActiveCameraPreset(gViewport, 6); break;
    case 'f':
    case 'F':
        MirEngineFitViewport(gViewport);
        printf("[fit] viewport fitted\n");
        break;
    case 'i':
    case 'I':
        printCameraInfo();
        break;
    case 'd':
    case 'D':
        printOpenGLDiagnostics();
        break;
    case 's':
    case 'S':
        saveFrameAsPPM("/tmp/dv_capture.ppm");
        break;
    case 'e':
    case 'E':
        printLastError();
        break;
    case 'b':
    case 'B':
        createBox();
        break;
    case 'c':
    case 'C':
    {
        const NSPoint point = [self enginePoint:event];
        MirEngineViewportClick(gViewport, (float)point.x, (float)point.y, false);
        const uint64_t selected = MirEngineGetSelectedObjectId(gViewport);
        printf("[pick] selected id = %llu\n", static_cast<unsigned long long>(selected));
        break;
    }
    default:
        break;
    }
}

@end

// =================================================================================
// main — окно + run loop + smoke
// =================================================================================

int main(int argc, const char* argv[])
{
    setvbuf(stdout, nullptr, _IONBF, 0);

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--smoke") == 0)
        {
            gSmoke = true;
            if (i + 1 < argc)
            {
                const int parsed = atoi(argv[i + 1]);
                if (parsed > 0)
                {
                    gSmokeFrames = parsed;
                    ++i;
                }
            }
        }
        else if (std::strcmp(argv[i], "--autoshot") == 0)
        {
            // GUI-режим с показанным окном: рендер кадров на дистанциях 12 и
            // 200 с сохранением PPM — валидные кадры реального backbuffer.
            gAutoShot = true;
        }
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: MIR4D_DebugViewport [--smoke [frames]] [--autoshot]\n");
            return 0;
        }
    }

    @autoreleasepool
    {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // ------------------------------------------------------------
        // Окно и view
        // ------------------------------------------------------------
        const NSRect contentRect = NSMakeRect(0, 0, 1024, 768);
        NSWindow* window = [[NSWindow alloc]
            initWithContentRect:contentRect
                      styleMask:NSWindowStyleMaskTitled |
                                NSWindowStyleMaskClosable |
                                NSWindowStyleMaskMiniaturizable |
                                NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                          defer:NO];
        [window setTitle:@"MIR 4D Debug Viewport"];
        [window center];

        MIR4DDebugView* view = [[MIR4DDebugView alloc]
            initWithFrame:NSMakeRect(0, 0, contentRect.size.width, contentRect.size.height)];
        [view setWantsLayer:YES];
        [window setContentView:view];
        gView = view;

        const NSRect backing = [view convertRectToBacking:view.bounds];
        const MirEngineSize2D size{
            static_cast<uint32_t>(std::max(backing.size.width, 1.0)),
            static_cast<uint32_t>(std::max(backing.size.height, 1.0))};

        // ------------------------------------------------------------
        // Engine через C ABI (тот же путь, что и MirUI)
        // ------------------------------------------------------------
        void* context = MirEngineCreateMacOpenGLContext((__bridge void*)view, size);
        if (!context)
        {
            printf("FATAL: failed to create OpenGL context\n");
            return 1;
        }

        gRenderer = MirEngineCreateOpenGLRenderer(context);
        if (!gRenderer || !MirEngineInitializeRenderer(gRenderer))
        {
            printf("FATAL: failed to create/initialize renderer\n");
            return 1;
        }

        gViewport = MirEngineCreateViewport(gRenderer, size.width, size.height);
        if (!gViewport)
        {
            printf("FATAL: failed to create viewport\n");
            return 1;
        }

        if (!gSmoke)
        {
            if (!createBox())
                return 1;

            printHelp();
            printCameraInfo();

            [window makeKeyAndOrderFront:nil];
            [app activateIgnoringOtherApps:YES];
            [window makeFirstResponder:view];
        }

        // ------------------------------------------------------------
        // Run loop / smoke
        // ------------------------------------------------------------
        int frame = 0;
        double lastPrint = 0.0;
        double fpsAccum = 0.0;
        int fpsFrames = 0;

        const double distancesToProbe[] = {12.0, 50.0, 200.0, 1000.0, 5000.0};

        while (!gQuit)
        {
            NSEvent* event = [app nextEventMatchingMask:NSEventMaskAny
                                              untilDate:[NSDate dateWithTimeIntervalSinceNow:1.0 / 60.0]
                                                 inMode:NSDefaultRunLoopMode
                                                dequeue:YES];
            [app sendEvent:event];

            renderFrame();
            ++frame;

            if (gSmoke)
            {
                // Обход дистанций: печатаем состояние и рендерим кадры.
                const int perDistance = gSmokeFrames / 5;
                const int probeIndex = std::min(frame / perDistance, 4);
                if (frame % 30 == 0 || frame == gSmokeFrames - 1)
                {
                    const double distance = distancesToProbe[probeIndex];
                    float theta = 0.0f, phi = 0.0f, actual = 0.0f;
                    MirEngineGetCameraOrientation(gViewport, &theta, &phi, &actual);
                    double eyeX = 0.0, eyeY = 0.0, eyeZ = 0.0;
                    computeEye(theta, phi, actual, eyeX, eyeY, eyeZ);
                    double nearPlane = 0.0, farPlane = 0.0;
                    computeClipping(actual, nearPlane, farPlane);
                    printf("[smoke] frame=%d  theta=%.4f  phi=%.4f  distance=%.3f  "
                           "eye=(%.3f, %.3f, %.3f)  near=%.6f  far=%.3f\n",
                           frame, (double)theta, (double)phi, (double)actual,
                           eyeX, eyeY, eyeZ, nearPlane, farPlane);
                }

                // Захват кадров на каждой дистанции.
                if (frame % perDistance == perDistance / 2)
                {
                    char path[128];
                    std::snprintf(path, sizeof(path), "/tmp/dv_frame_%d.ppm",
                                  probeIndex + 1);
                    saveFrameAsPPM(path);
                }

                if (frame == 60)
                    MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 12.0f);
                if (frame == 100)
                    MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 50.0f);
                if (frame == 200)
                    MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 200.0f);
                if (frame == 400)
                    MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 1000.0f);
                if (frame == 800)
                    MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 5000.0f);

                if (frame >= gSmokeFrames)
                    break;
            }
            else
            {
                if (gAutoShot)
                {
                    // Дистанция 12: кадр на 90-м кадре.
                    if (frame == 90)
                        saveFrameAsPPM("/tmp/dv_gui_d12.ppm");
                    if (frame == 120)
                        MirEngineSetCameraOrientation(gViewport, 0.8f, 1.2f, 200.0f);
                    // Дистанция 200: кадр на 210-м кадре, затем выход.
                    if (frame == 210)
                    {
                        saveFrameAsPPM("/tmp/dv_gui_d200.ppm");
                        gQuit = true;
                    }
                }
                else
                {
                    fpsAccum += 1.0;
                    ++fpsFrames;
                    const double now = CFAbsoluteTimeGetCurrent();
                    if (now - lastPrint >= 5.0)
                    {
                        printf("[fps] %.1f fps over %d frames\n",
                               fpsFrames / (now - lastPrint), fpsFrames);
                        lastPrint = now;
                        fpsFrames = 0;
                        fpsAccum = 0.0;
                    }
                }
            }
        }

        MirEngineDestroyViewport(gViewport);
        MirEngineDestroyRenderer(gRenderer);
        MirEngineDestroyOpenGLContext(context);

        printf("MIR 4D Debug Viewport: closed\n");
    }

    return 0;
}
