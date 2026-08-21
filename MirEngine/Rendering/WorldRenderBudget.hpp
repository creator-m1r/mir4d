#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace mir
{

enum class RenderQualityLevel : std::uint8_t
{
    Performance,
    Balanced,
    Quality,
    Cinematic
};

struct WorldRenderBudget
{
    std::size_t particleBudget{16384};
    std::size_t lightBudget{128};
    std::size_t shadowCasterBudget{64};
    std::size_t physicsBudget{4096};
    std::size_t audioSourceBudget{128};

    float targetFrameTimeMs{16.6667F};
    float qualityScale{1.0F};

    static WorldRenderBudget forQuality(RenderQualityLevel quality) noexcept
    {
        switch (quality)
        {
        case RenderQualityLevel::Performance:
            return {4096, 32, 16, 1024, 32, 20.0F, 0.60F};
        case RenderQualityLevel::Balanced:
            return {16384, 128, 64, 4096, 128, 16.6667F, 1.0F};
        case RenderQualityLevel::Quality:
            return {32768, 256, 128, 8192, 256, 16.6667F, 1.35F};
        case RenderQualityLevel::Cinematic:
            return {65536, 512, 256, 16384, 512, 16.6667F, 1.75F};
        }
        return {};
    }

    void adapt(float frameTimeMs) noexcept
    {
        if (frameTimeMs > targetFrameTimeMs * 1.15F)
            qualityScale = std::max(0.50F, qualityScale * 0.92F);
        else if (frameTimeMs < targetFrameTimeMs * 0.80F)
            qualityScale = std::min(1.75F, qualityScale * 1.04F);

        particleBudget = scaledBudget(particleBudget, qualityScale);
        lightBudget = scaledBudget(lightBudget, qualityScale);
        shadowCasterBudget = scaledBudget(shadowCasterBudget, qualityScale);
        physicsBudget = scaledBudget(physicsBudget, qualityScale);
        audioSourceBudget = scaledBudget(audioSourceBudget, qualityScale);
    }

private:
    [[nodiscard]] static std::size_t scaledBudget(
        std::size_t base,
        float scale) noexcept
    {
        return std::max<std::size_t>(1, static_cast<std::size_t>(base * scale));
    }
};

}
