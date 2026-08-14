// MirEngine/Rendering/OpenGL/OpenGLDebug.cpp
// =================================================================================
// Реализация отладочных утилит OpenGL.
// Использует glGetError и GL_KHR_debug для мониторинга состояния GPU.
// =================================================================================

#include "OpenGLDebug.h"
#include <string>
#include <sstream>

#include "../../Core/Logging/Logger.hpp" // для ::mir::globalLogger()

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
        default: {
            std::ostringstream ss;
            ss << "Unknown error 0x" << std::hex << std::uppercase << static_cast<unsigned int>(err);
            msg = ss.str();
            break;
        }
    }

    std::string full = std::string("[GL Debug] Error") + (context.empty() ? "" : (" (" + context + ")")) + ": " + msg;
    ::mir::globalLogger().error(full);
    return false;
}

bool OpenGLDebug::enableDebugOutput()
{
#ifdef GL_DEBUG_OUTPUT
    // Важно: загрузчик GL (glad/glbinding) должен быть инициализирован до этого вызова.
    // Попытка использовать функции GL до создания контекста/инициализации загрузчика — небезопасна.
    // Проверим, есть ли GL-контекст и версия:
    const GLubyte* verStr = glGetString(GL_VERSION);
    if (!verStr) {
        ::mir::globalLogger().warning("[GL Debug] No GL context or glGetString returned null. Cannot enable debug output safely.");
        return false;
    }

    // Попытка определить поддержку современного API для перечисления расширений.
    // Если версия >= 3.0, используем glGetStringi / GL_NUM_EXTENSIONS, иначе fallback на glGetString(GL_EXTENSIONS).
    bool supported = false;
    {
        // Парсим основной номер версии (формат, например, "4.6.0 NVIDIA 450.66")
        int major = 0;
        std::string vs = reinterpret_cast<const char*>(verStr);
        if (!vs.empty() && std::isdigit(static_cast<unsigned char>(vs[0]))) {
            major = vs[0] - '0';
        }

        if (major >= 3) {
            GLint numExt = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
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
        } else {
            // fallback: одна строка с пробел-разделёнными расширениями
            const GLubyte* exts = glGetString(GL_EXTENSIONS);
            if (exts) {
                std::string s = reinterpret_cast<const char*>(exts);
                if (s.find("GL_KHR_debug") != std::string::npos ||
                    s.find("GL_ARB_debug_output") != std::string::npos) {
                    supported = true;
                }
            }
        }
    }

    if (!supported) {
        ::mir::globalLogger().warning("[GL Debug] KHR_debug / ARB_debug_output not supported");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    // Включаем все сообщения
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    ::mir::globalLogger().info("[GL Debug] Debug output enabled");
    return true;
#else
    ::mir::globalLogger().warning("[GL Debug] GL_DEBUG_OUTPUT not available in this header");
    return false;
#endif
}

#ifdef GL_DEBUG_OUTPUT
void OpenGLDebug::debugCallback(GLenum /*source*/,
                                GLenum /*type*/,
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

    std::ostringstream ss;
    ss << "[GL Debug][" << sev << "] (id=" << id << ") " << (message ? message : "");
    ::mir::globalLogger().warning(ss.str());
}
#endif

} // namespace Rendering
} // namespace MirEngine
