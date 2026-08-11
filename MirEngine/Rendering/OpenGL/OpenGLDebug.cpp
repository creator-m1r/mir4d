// MirEngine/Rendering/OpenGL/OpenGLDebug.cpp
// =================================================================================
// Реализация отладочных утилит OpenGL.
// Использует glGetError и GL_KHR_debug для мониторинга состояния GPU.
// =================================================================================


#include "OpenGLDebug.h"
#include <iostream>
#include <string>

namespace MirEngine {
namespace Rendering {

bool OpenGLDebug::checkError(const std::string& context)
{
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return true;
    }

    std::string msg;
    switch (err) {
        case GL_INVALID_ENUM:      msg = "GL_INVALID_ENUM";      break;
        case GL_INVALID_VALUE:     msg = "GL_INVALID_VALUE";     break;
        case GL_INVALID_OPERATION: msg = "GL_INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:     msg = "GL_OUT_OF_MEMORY";     break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            msg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        default:                   msg = "Unknown error 0x" + std::to_string(err);
    }

    std::cerr << "[GL Debug] Error" << (context.empty() ? "" : " (" + context + ")")
              << ": " << msg << "\n";
    return false;
}

bool OpenGLDebug::enableDebugOutput()
{
#ifdef GL_DEBUG_OUTPUT
    // Проверяем поддержку
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    bool supported = false;
    for (GLint i = 0; i < numExt; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
        if (ext) {
            std::string name = reinterpret_cast<const char*>(ext);
            if (name == "GL_KHR_debug" || name == "GL_ARB_debug_output") {
                supported = true;
                break;
            }
        }
    }

    if (!supported) {
        std::cerr << "[GL Debug] KHR_debug / ARB_debug_output not supported\n";
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    // Включаем все сообщения
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    std::cout << "[GL Debug] Debug output enabled\n";
    return true;
#else
    std::cerr << "[GL Debug] GL_DEBUG_OUTPUT not available in this header\n";
    return false;
#endif
}

#ifdef GL_DEBUG_OUTPUT
void OpenGLDebug::debugCallback(GLenum /*source*/,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei /*length*/,
                                const GLchar* message,
                                const void* /*userParam*/)
{
    // Игнорируем уведомления
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    std::string sev;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   sev = "HIGH";   break;
        case GL_DEBUG_SEVERITY_MEDIUM: sev = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:    sev = "LOW";    break;
        default:                       sev = "UNKNOWN";
    }

    std::cerr << "[GL Debug][" << sev << "] (id=" << id << ") " << message << "\n";
}
#endif

} // namespace Rendering
} // namespace MirEngine