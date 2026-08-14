#pragma once

#include "../Resources/RenderMeshGPU.hpp"

namespace MirEngine {
namespace Rendering {

struct OpenGLDrawParameters
{
    bool wireframe{false};
};

class OpenGLMeshRenderer
{
public:
    void draw(const RenderMeshGPU& mesh,
              const OpenGLDrawParameters& parameters = {}) const noexcept;
};

} // namespace Rendering
} // namespace MirEngine
