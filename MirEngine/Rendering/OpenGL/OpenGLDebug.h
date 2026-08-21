// MirEngine/Rendering/OpenGL/OpenGLDebug.h
// =================================================================================
// Диагностический модуль OpenGL.
//
// Предоставляет:
//   - информацию о контексте и устройстве (vendor, renderer, version, GLSL,
//     profile, extensions);
//   - лимиты реализации (texture size, vertex attribs, uniform vectors,
//     line width range, viewport dims, samples);
//   - проверку ошибок после вызовов GL-функций (checkError);
//   - сброс накопленных ошибок (resetErrors);
//   - синхронный вывод отладочных сообщений драйвера (GL_KHR_debug);
//   - текстовый отчёт buildReport / logReport для системной диагностики.
//
// Архитектура:
//   - Не имеет состояния (все методы статические).
//   - Вызывается из любого места OpenGL-слоя после подозрительных вызовов.
//   - Требует текущего OpenGL-контекста.
//
// Использование:
//   OpenGLDebug::logReport();          // полный отчёт в лог
//   glSomeCall();
//   OpenGLDebug::checkError("after glSomeCall");
// =================================================================================

#pragma once

#include <string>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

/// Информация об устройстве и контексте OpenGL.
struct OpenGLDeviceInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string glslVersion;
    std::string profile;   ///< "Core Profile", "Compatibility Profile" и т.п.
    int major = 0;
    int minor = 0;
};

/// Лимиты реализации, влияющие на рендеринг.
struct OpenGLLimits {
    GLint maxTextureSize = 0;
    GLint maxVertexAttribs = 0;
    GLint maxVertexUniformVectors = 0;
    GLint maxFragmentUniformVectors = 0;
    GLint maxCombinedTextureUnits = 0;
    GLint maxSamples = 0;
    GLfloat lineWidthRange[2] = {1.0f, 1.0f};
    GLint maxViewportDims[2] = {0, 0};
};

class OpenGLDebug {
public:
    // Проверяет glGetError после вызова. Возвращает true, если ошибок нет.
    static bool checkError(const std::string& context = "");

    // Сбрасывает накопленные ошибки (glGetError до GL_NO_ERROR).
    static void resetErrors();

    // Информация об устройстве и контексте.
    static OpenGLDeviceInfo getDeviceInfo();

    // Лимиты реализации.
    static OpenGLLimits getLimits();

    // Список расширений контекста.
    static std::vector<std::string> getExtensions();

    // Полный текстовый отчёт о контексте OpenGL.
    static std::string buildReport();

    // Выводит отчёт в глобальный лог (::mir::globalLogger).
    static void logReport();

    // Включает синхронный вывод сообщений драйвера (GL_KHR_debug / ARB).
    // Возвращает true при успехе.
    static bool enableDebugOutput();

private:
#ifdef GL_DEBUG_OUTPUT
    static void debugCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* message,
                              const void* userParam);
#endif
};

} // namespace Rendering
} // namespace MirEngine