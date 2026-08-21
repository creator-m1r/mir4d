#include "MirUIExports.h"
#include "../Application/CADApplication.hpp"
#include <memory>
#include <string>

static std::unique_ptr<MirUI::CADApplication> g_app;

extern "C" {

bool MirUI_CAD_Initialize(void) {
    if (g_app) return true;
    g_app = std::make_unique<MirUI::CADApplication>();
    return g_app->initialize();
}

void MirUI_CAD_Shutdown(void) {
    if (g_app) {
        g_app->shutdown();
        g_app.reset();
    }
}

void MirUI_CAD_Update(double deltaTime) {
    if (g_app) g_app->update(deltaTime);
}

void MirUI_CAD_Render(void) {
    if (g_app) g_app->render();
}

void MirUI_CAD_LoadModel(const char* path) {
    if (g_app && path) g_app->loadModel(path);
}

void MirUI_CAD_SelectObject(const char* id) {
    if (g_app && id) g_app->selectObject(id);
}

} // extern "C"