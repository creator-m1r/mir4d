#pragma once

namespace mir
{
class Scene;
}

namespace MirEngine::Rendering
{

class RenderContext;
class RenderDevice;

/// Backend-neutral render pass contract.
/// Camera-derived matrices and frame data arrive through RenderContext;
/// passes never depend on a concrete camera implementation.
class RenderPass
{
public:
    virtual ~RenderPass() = default;

    virtual void begin(RenderContext& context, RenderDevice* device = nullptr)
    {
        (void)context;
        (void)device;
    }

    virtual void execute(RenderContext& context,
                         mir::Scene& scene,
                         RenderDevice& device) = 0;

    virtual void end(RenderContext& context, RenderDevice* device = nullptr)
    {
        (void)context;
        (void)device;
    }

protected:
    RenderPass() = default;
};

} // namespace MirEngine::Rendering
