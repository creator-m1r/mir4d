// MirEngine/Rendering/OpenGL/OpenGLDebug.cpp
// =================================================================================
// Реализация диагностического модуля OpenGL.
// Использует glGetError, glGetString/glGetStringi и GL_KHR_debug
// для мониторинга состояния GPU и контекста.
// =================================================================================

#include "OpenGLDebug.h"

#include <cctype>
#include <cstring>
#include <sstream>

#include "../../Core/Logging/Logger.hpp" // для ::mir::globalLogger()

namespace MirEngine {
namespace Rendering {

namespace {

std::string glString(GLenum name)
{
    const GLubyte* s = glGetString(name);
    return s ? reinterpret_cast<const char*>(s) : std::string();
}

int parseMajorVersion(const std::string& version)
{
    if (version.empty() || !std::isdigit(static_cast<unsigned char>(version[0])))
        return 0;
    return version[0] - '0';
}

} // namespace

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
            ss << "Unknown error 0x" << std::hex << std::uppercase
               << static_cast<unsigned int>(err);
            msg = ss.str();
            break;
        }
    }

    std::string full = std::string("[GL Debug] Error")
        + (context.empty() ? "" : (" (" + context + ")")) + ": " + msg;
    ::mir::globalLogger().error(full);
    return false;
}

void OpenGLDebug::resetErrors()
{
    while (glGetError() != GL_NO_ERROR) {
    }
}

OpenGLDeviceInfo OpenGLDebug::getDeviceInfo()
{
    OpenGLDeviceInfo info;
    info.vendor     = glString(GL_VENDOR);
    info.renderer   = glString(GL_RENDERER);
    info.version    = glString(GL_VERSION);
    info.glslVersion = glString(GL_SHADING_LANGUAGE_VERSION);

    const std::string& v = info.version;
    if (v.find("Core Profile") != std::string::npos)
        info.profile = "Core Profile";
    else if (v.find("Compatibility Profile") != std::string::npos)
        info.profile = "Compatibility Profile";
    else
    {
        // macOS не добавляет суффикс в строку версии; определяем профиль
        // через GL_CONTEXT_PROFILE_MASK (3.2+).
        GLint mask = 0;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
        if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
            info.profile = "Compatibility";
        else if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
            info.profile = "Core";
        else
            info.profile = "Legacy";
    }

    info.major = parseMajorVersion(v);

    // minor: первая цифра после "major."
    if (v.size() >= 3 && v[1] == '.' &&
        std::isdigit(static_cast<unsigned char>(v[2]))) {
        info.minor = v[2] - '0';
    }
    return info;
}

OpenGLLimits OpenGLDebug::getLimits()
{
    OpenGLLimits limits;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limits.maxTextureSize);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &limits.maxVertexAttribs);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &limits.maxVertexUniformVectors);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &limits.maxFragmentUniformVectors);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &limits.maxCombinedTextureUnits);
    glGetIntegerv(GL_MAX_SAMPLES, &limits.maxSamples);
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, limits.lineWidthRange);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, limits.maxViewportDims);
    return limits;
}

std::vector<std::string> OpenGLDebug::getExtensions()
{
    std::vector<std::string> result;
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    if (numExt <= 0)
        return result;
    result.reserve(static_cast<std::size_t>(numExt));
    for (GLint i = 0; i < numExt; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
        if (ext)
            result.emplace_back(reinterpret_cast<const char*>(ext));
    }
    return result;
}

std::string OpenGLDebug::buildReport()
{
    const OpenGLDeviceInfo info = getDeviceInfo();
    const OpenGLLimits limits = getLimits();
    const std::vector<std::string> extensions = getExtensions();

    std::ostringstream ss;
    ss << "[GL Diagnostics] ---- OpenGL context report ----\n";
    ss << "[GL Diagnostics] Vendor:   " << info.vendor << "\n";
    ss << "[GL Diagnostics] Renderer: " << info.renderer << "\n";
    ss << "[GL Diagnostics] Version:  " << info.version
       << " (OpenGL " << info.major << "." << info.minor << ", " << info.profile << ")\n";
    ss << "[GL Diagnostics] GLSL:     " << info.glslVersion << "\n";
    ss << "[GL Diagnostics] Extensions: " << extensions.size() << "\n";
    ss << "[GL Diagnostics] Limits:\n";
    ss << "[GL Diagnostics]   MaxTextureSize            = " << limits.maxTextureSize << "\n";
    ss << "[GL Diagnostics]   MaxVertexAttribs          = " << limits.maxVertexAttribs << "\n";
    ss << "[GL Diagnostics]   MaxVertexUniformVectors   = " << limits.maxVertexUniformVectors << "\n";
    ss << "[GL Diagnostics]   MaxFragmentUniformVectors = " << limits.maxFragmentUniformVectors << "\n";
    ss << "[GL Diagnostics]   MaxCombinedTextureUnits   = " << limits.maxCombinedTextureUnits << "\n";
    ss << "[GL Diagnostics]   MaxSamples                = " << limits.maxSamples << "\n";
    ss << "[GL Diagnostics]   AliasedLineWidthRange     = ["
       << limits.lineWidthRange[0] << ", " << limits.lineWidthRange[1] << "]\n";
    ss << "[GL Diagnostics]   MaxViewportDims           = ["
       << limits.maxViewportDims[0] << ", " << limits.maxViewportDims[1] << "]\n";
    for (const auto& ext : extensions)
        ss << "[GL Diagnostics]   Ext: " << ext << "\n";
    ss << "[GL Diagnostics] ---- end of report ----";
    return ss.str();
}

void OpenGLDebug::logReport()
{
    const std::string report = buildReport();
    std::istringstream in(report);
    std::string line;
    while (std::getline(in, line))
        ::mir::globalLogger().info(line);
}

bool OpenGLDebug::enableDebugOutput()
{
#ifdef GL_DEBUG_OUTPUT
    const GLubyte* verStr = glGetString(GL_VERSION);
    if (!verStr) {
        ::mir::globalLogger().warning("[GL Debug] No GL context or glGetString returned null. Cannot enable debug output safely.");
        return false;
    }

    bool supported = false;
    {
        const int major = parseMajorVersion(reinterpret_cast<const char*>(verStr));

        if (major >= 3) {
            GLint numExt = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
            for (GLint i = 0; i < numExt; ++i) {
                const GLubyte* ext = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
                if (ext) {
                    const std::string name = reinterpret_cast<const char*>(ext);
                    if (name == "GL_KHR_debug" || name == "GL_ARB_debug_output") {
                        supported = true;
                        break;
                    }
                }
            }
        } else {
            const std::string exts = glString(GL_EXTENSIONS);
            if (exts.find("GL_KHR_debug") != std::string::npos ||
                exts.find("GL_ARB_debug_output") != std::string::npos) {
                supported = true;
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