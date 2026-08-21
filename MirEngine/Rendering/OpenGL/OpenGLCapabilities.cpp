
#include "OpenGLCapabilities.h"

#include <iostream>
#include <cstring>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine {
namespace Rendering {

CapabilitiesInfo OpenGLCapabilities::query()
{
    CapabilitiesInfo info;

    const GLubyte* vendor   = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    const GLubyte* shading  = glGetString(GL_SHADING_LANGUAGE_VERSION);

    info.vendor                  = vendor   ? reinterpret_cast<const char*>(vendor)   : "Unknown";
    info.renderer                = renderer ? reinterpret_cast<const char*>(renderer) : "Unknown";
    info.version                 = version  ? reinterpret_cast<const char*>(version)  : "Unknown";
    info.shadingLanguageVersion  = shading  ? reinterpret_cast<const char*>(shading)  : "Unknown";

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &info.maxTextureSize);
    glGetIntegerv(GL_MAX_SAMPLES,      &info.maxSamples);

    info.anisotropySupported =
        isExtensionSupported("GL_EXT_texture_filter_anisotropic") ||
        isExtensionSupported("GL_ARB_texture_filter_anisotropic");

    if (info.anisotropySupported) {
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &info.maxAnisotropy);
#endif
    }

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLint i = 0; i < numExtensions; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
        if (ext) {
            info.extensions.emplace_back(reinterpret_cast<const char*>(ext));
        }
    }

    std::cout << "[OpenGLCapabilities] Vendor   : " << info.vendor   << "\n"
              << "[OpenGLCapabilities] Renderer : " << info.renderer << "\n"
              << "[OpenGLCapabilities] Version  : " << info.version  << "\n"
              << "[OpenGLCapabilities] GLSL     : " << info.shadingLanguageVersion << "\n"
              << "[OpenGLCapabilities] Max Tex  : " << info.maxTextureSize << "\n"
              << "[OpenGLCapabilities] Max MSAA : " << info.maxSamples << "\n"
              << "[OpenGLCapabilities] Aniso    : " << (info.anisotropySupported ? "yes" : "no")
              << " (max " << info.maxAnisotropy << ")\n"
              << "[OpenGLCapabilities] Exts     : " << info.extensions.size() << "\n";

    return info;
}

bool OpenGLCapabilities::isExtensionSupported(const std::string& name)
{
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    for (GLint i = 0; i < numExt; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));
        if (ext && name == reinterpret_cast<const char*>(ext)) {
            return true;
        }
    }
    return false;
}

}
}