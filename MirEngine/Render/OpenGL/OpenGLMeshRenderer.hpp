#pragma once

#include "../RenderMeshGPU.hpp"

namespace mir
{

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

} // namespace mir
