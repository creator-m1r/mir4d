
#pragma once

#include <string>
#include <vector>

namespace MirEngine {
namespace Rendering {

struct CapabilitiesInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string shadingLanguageVersion;

    int  maxTextureSize     = 0;
    int  maxSamples         = 0;
    int  maxAnisotropy      = 1;
    bool anisotropySupported = false;

    std::vector<std::string> extensions;
};

class OpenGLCapabilities {
public:

    static CapabilitiesInfo query();

    static bool isExtensionSupported(const std::string& name);
};

}
}