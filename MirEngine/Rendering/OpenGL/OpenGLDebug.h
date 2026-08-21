
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

struct OpenGLDeviceInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string glslVersion;
    std::string profile;
    int major = 0;
    int minor = 0;
};

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

    static bool checkError(const std::string& context = "");

    static void resetErrors();

    static OpenGLDeviceInfo getDeviceInfo();

    static OpenGLLimits getLimits();

    static std::vector<std::string> getExtensions();

    static std::string buildReport();

    static void logReport();

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

}
}