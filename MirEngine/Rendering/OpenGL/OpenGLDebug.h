// MirEngine/Rendering/OpenGL/OpenGLDebug.h
// =================================================================================
// Утилита отладки OpenGL.
//
// Предоставляет статические методы для проверки ошибок после вызовов GL-функций
// (checkError) и для включения синхронного вывода отладочных сообщений драйвера
// (enableDebugOutput). Вся функциональность реализована через glbinding и
// стандартный механизм GL_KHR_debug (OpenGL 4.3+), который доступен в 4.1
// через расширение.
//
// Архитектура:
//   - Не имеет состояния (все методы статические) для простоты использования.
//   - Вызывается из любого места OpenGL-слоя после подозрительных вызовов.
//   - Логирует ошибки и предупреждения через spdlog с префиксом [GL Debug].
//
// Использование:
//   OpenGLDebug::enableDebugOutput(); // в начале инициализации
//   ...
//   glSomeCall();
//   OpenGLDebug::checkError("after glSomeCall");
// =================================================================================


#pragma once

#include <string>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

class OpenGLDebug {
public:
    // Проверяет glGetError после вызова. Возвращает true, если ошибок нет.
    static bool checkError(const std::string& context = "");

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