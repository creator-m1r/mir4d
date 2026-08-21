#pragma once

namespace mir
{
class Scene;
}

namespace MirEngine::Rendering
{

class RenderContext;
class RenderDevice;

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

}
